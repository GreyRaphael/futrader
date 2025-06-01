#pragma once

#include <parallel_hashmap/phmap.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <format>
#include <optional>
#include <print>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "config_parser.h"
#include "i_order.h"
#include "i_quote.h"
#include "stra_cta.h"
#include "tick_parser.hpp"
#include "zmq.h"

// RAII wrapper for ZeroMQ context and sockets
struct ZmqContext {
    ZmqContext(int io_threads = 1) {
        _ctx = zmq_ctx_new();
        assert(_ctx);
        zmq_ctx_set(_ctx, ZMQ_IO_THREADS, io_threads);
    }
    ~ZmqContext() { zmq_ctx_destroy(_ctx); }
    void* get() const noexcept { return _ctx; }

   private:
    void* _ctx;
};

struct ZmqSocket {
    ZmqSocket() noexcept : _socket(nullptr) {}
    ZmqSocket(void* ctx, int type, std::string_view addr, bool bind = false) {
        _socket = zmq_socket(ctx, type);
        assert(_socket);
        if (bind)  // as listener
            zmq_bind(_socket, addr.data());
        else  // as client
            zmq_connect(_socket, addr.data());
    }
    ~ZmqSocket() {
        if (_socket) zmq_close(_socket);
    }
    void* get() const noexcept { return _socket; }

   private:
    void* _socket;
};

struct CtaStgWorker {
    CtaStgWorker(std::vector<std::string>& symbols, std::string_view task_url, void* context) : _context(context) {
        _work_pull_socket = ZmqSocket(_context, ZMQ_PULL, task_url, false);
        _order_push_socket = ZmqSocket(_context, ZMQ_PUSH, "ipc://@orders", true);
        _stg_map.reserve(symbols.size());
    }

    void addStrategy(std::string_view symbol, CtaStrategy strategy) {
        _stg_map[symbol].emplace_back(std::move(strategy));
    }

    void start(std::stop_token stop) {
        TickData tick{};
        zmq_pollitem_t items[] = {{_work_pull_socket.get(), 0, ZMQ_POLLIN, 0}};
        while (!stop.stop_requested()) {
            zmq_poll(items, 1, 100);  // Wait 100ms at most for socket
            if (items[0].revents & ZMQ_POLLIN &&
                zmq_recv(_work_pull_socket.get(), &tick, sizeof(TickData), 0) > 0) {
                if (_stg_map.contains(tick.symbol)) {
                    for (auto&& stg : _stg_map[tick.symbol]) {
                        // visit strategy with the tick data
                        if (auto order = std::visit([&tick](auto&& strategy) -> std::optional<Order> { return strategy.onTick(tick); }, stg)) {
                            // send to another
                            zmq_send(_order_push_socket.get(), &order.value(), sizeof(Order), 0);
                        }
                    }
                } else {
                    std::println("Worker received tick for unknown symbol: {}", tick.symbol);
                }
            }
        }
    }

   private:
    void* _context;
    ZmqSocket _work_pull_socket{}, _order_push_socket{};
    phmap::flat_hash_map<std::string, std::vector<CtaStrategy>> _stg_map;

    std::atomic<bool> _running{true};
};

template <size_t ThreadNum>
struct CtaZmqEngine {
    CtaZmqEngine(std::string_view cfg_filename) {
        assert(std::filesystem::exists(cfg_filename));
        auto config = ZmqConfig::readConfig(cfg_filename);

        _split2buckets(config.symbols);
        _stg_map.reserve(config.symbols.size());

        // subscribe to market data
        _md_sub_socket = ZmqSocket(_context.get(), ZMQ_SUB, config.address, false);
        int rcvhwm = 0;  // Adjust as needed
        zmq_setsockopt(_md_sub_socket.get(), ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
        for (auto&& topic : config.symbols) {
            zmq_setsockopt(_md_sub_socket.get(), ZMQ_SUBSCRIBE, topic.data(), topic.length());
        }
    }

    void addStrategy(std::string_view symbol, CtaStrategy strategy) {
        _stg_map[symbol].emplace_back(std::move(strategy));
    }

    void init() {
        for (auto i = 0; i < ThreadNum; ++i) {
            auto addr = std::format("inproc://worker-{}", i);
            _work_push_sockets[i] = ZmqSocket(_context.get(), ZMQ_PUSH, addr, true);
            _worker_threads[i] = std::jthread([this, i, addr](std::stop_token stoken) {
                CtaStgWorker worker{_buckets[i], addr, _context.get()};
                // maybe register strategies:
                for (auto&& symbol : _buckets[i]) {
                    if (_stg_map.contains(symbol)) {
                        for (auto&& stg : _stg_map[symbol]) {
                            worker.addStrategy(symbol, std::move(stg));
                        }
                    } else {
                        std::println("Worker {} received unknown symbol: {}", i, symbol);
                    }
                }

                worker.start(stoken);
            });
        }
    }

    void start() {
        TickData tick{};
        zmq_pollitem_t items[] = {{_md_sub_socket.get(), 0, ZMQ_POLLIN, 0}};
        while (_running.load(std::memory_order_relaxed)) {
            zmq_poll(items, 1, 100);  // Wait 100ms at most for socket
            if (items[0].revents & ZMQ_POLLIN &&
                zmq_recv(_md_sub_socket.get(), &tick, sizeof(TickData), 0) > 0) {
                auto worker_id = hashSymbol(tick.symbol) % ThreadNum;
                zmq_send(_work_push_sockets[worker_id], &tick, sizeof(TickData), 0);
            }
        }
    }

    void stop() {
        _running.store(false, std::memory_order_relaxed);
        // also signal each worker
        for (auto& worker : _worker_threads) {
            if (worker.joinable()) {
                worker.request_stop();
            }
        }
    }

   private:
    void _split2buckets(std::span<std::string_view> symbols) {
        std::array<size_t, ThreadNum> counts{};
        // Count
        for (auto sv : symbols) counts[hashSymbol(sv) % ThreadNum]++;
        // Reserve
        for (int i = 0; i < ThreadNum; ++i) _buckets[i].reserve(counts[i]);
        // Fill
        for (auto sv : symbols) _buckets[hashSymbol(sv) % ThreadNum].emplace_back(sv);
    }

    ZmqContext _context{1};  // Single IO thread for simplicity
    ZmqSocket _md_sub_socket{};
    std::array<ZmqSocket, ThreadNum> _work_push_sockets{};
    std::array<std::jthread, ThreadNum> _worker_threads{};

    std::array<std::vector<std::string>, ThreadNum> _buckets{};
    phmap::flat_hash_map<std::string, std::vector<CtaStrategy>> _stg_map;

    std::atomic<bool> _running{true};
};
