#pragma once

#include <parallel_hashmap/phmap.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "config_parser.h"
#include "i_order.h"
#include "i_quote.h"
#include "stra_cta.h"
#include "zmq.h"

struct CtaStgWorker {
    CtaStgWorker(std::vector<std::string>&& symbols, std::string_view task_url, void* context = nullptr) : _symbols(std::move(symbols)) {
        _context = (_context == nullptr) ? zmq_ctx_new() : context;

        _tick_puller = zmq_socket(_context, ZMQ_PULL);
        _order_pusher = zmq_socket(_context, ZMQ_PUSH);
        zmq_bind(_tick_puller, task_url.data());
        zmq_bind(_order_pusher, "ipc://@orders");
    }
    ~CtaStgWorker() {
        zmq_close(_tick_puller);
        zmq_close(_order_pusher);
    }

    void addStrategy(std::string_view symbol, CtaStrategy strategy) {
        _cta_stg_map[symbol].push_back(std::move(strategy));
    }

    void start() {
        TickData tick{};
        while (true) {
            // Receive messages
            if (zmq_recv(_tick_puller, &tick, sizeof(TickData), 0) <= 0) {
                // Handle error or exit condition
                continue;
            };

            for (auto&& stg : _cta_stg_map[tick.symbol]) {
                // visit strategy with the tick data
                auto order = std::visit([&tick](auto&& strategy) -> std::optional<Order> { return strategy.onTick(tick); }, stg);
                if (order) {
                    // send to another
                    zmq_send(_order_pusher, &order.value(), sizeof(Order), 0);
                }
            }
        }
    }

   private:
    std::vector<std::string> _symbols;
    void* _context{nullptr};
    void* _tick_puller{nullptr};
    void* _order_pusher{nullptr};
    phmap::flat_hash_map<std::string_view, std::vector<CtaStrategy>> _cta_stg_map;
};

template <size_t ThreadNum>
struct CtaZmqEngine {
    CtaZmqEngine(std::string_view cfg_filename) {
        assert(std::filesystem::exists(cfg_filename));
        auto config = ZmqConfig::readConfig(cfg_filename);

        _buckets = _split2buckets(config.symbols);

        _context = zmq_ctx_new();

        // subscribe to quotes
        _sub_socket = zmq_socket(_context, ZMQ_SUB);
        int rcvhwm = 0;  // Adjust as needed
        zmq_setsockopt(_sub_socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
        zmq_connect(_sub_socket, config.address.data());
        for (auto&& topic : config.symbols) {
            zmq_setsockopt(_sub_socket, ZMQ_SUBSCRIBE, topic.data(), topic.length());
        }
    }
    ~CtaZmqEngine() {
        zmq_close(_sub_socket);
        for (auto& socket : _push_sockets) {
            zmq_close(socket);
        }
        zmq_ctx_destroy(_context);
    }

    void init() {
        for (size_t i = 0; i < ThreadNum; ++i) {
            _push_sockets[i] = zmq_socket(_context, ZMQ_PUSH);
            auto addr = std::format("inproc://worker-{}", i);
            zmq_connect(_push_sockets[i], addr.data());
            _workers[i] = std::jthread([symbols = std::move(_buckets[i]), addr, this] {
                CtaStgWorker engine{std::move(symbols), addr, _context};
                for (auto&& [symbol, strategies] : _cta_stg_map) {
                    for (auto&& strategy : strategies) {
                        engine.addStrategy(symbol, std::move(strategy));
                    }
                }
                engine.start();
            });
        }
    }

    void start() {
        TickData tick{};
        while (true) {
            if (zmq_recv(_sub_socket, &tick, sizeof(TickData), 0) <= 0) {
                // Handle error or exit condition
                continue;
            }
            auto hash_value = _hasher(tick.symbol);
            auto worker_id = hash_value % ThreadNum;
            zmq_send(_push_sockets[worker_id], &tick, sizeof(TickData), 0);
        }
    }

    void addStrategy(std::string_view symbol, CtaStrategy strategy) {
        _cta_stg_map[symbol].push_back(std::move(strategy));
    }

   private:
    auto _split2buckets(std::span<const std::string_view> symbols) {
        std::array<size_t, ThreadNum> counts{};

        // Count
        for (auto sv : symbols) {
            counts[_hasher(sv) % ThreadNum]++;
        }

        // Reserve
        std::array<std::vector<std::string>, ThreadNum> buckets;
        for (int i = 0; i < ThreadNum; ++i) {
            buckets[i].reserve(counts[i]);
        }

        // Fill
        for (auto sv : symbols) {
            buckets[_hasher(sv) % ThreadNum].emplace_back(sv);
        }

        return buckets;
    }

    void* _context{nullptr};
    void* _sub_socket{nullptr};
    std::array<std::jthread, ThreadNum> _workers{};
    std::array<void*, ThreadNum> _push_sockets{};

    std::hash<std::string_view> _hasher{};
    std::array<std::vector<std::string>, ThreadNum> _buckets{};
    phmap::flat_hash_map<std::string_view, std::vector<CtaStrategy>> _cta_stg_map;
};
