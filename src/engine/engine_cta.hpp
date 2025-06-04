#pragma once

#include <parallel_hashmap/phmap.h>
#include <zmq.h>

#include <cassert>
#include <cstddef>
#include <optional>
#include <print>
#include <string_view>
#include <taskflow/taskflow.hpp>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "i_order.h"
#include "i_quote.h"
#include "mpsc.hpp"
#include "stra_cta.h"
#include "struct_parser.hpp"

// RAII Wrapper for ZeroMQ Context
struct ZmqContext {
    ZmqContext(int io_threads = 1) {
        _ctx = zmq_ctx_new();
        assert(_ctx);
        // Set I/O threads
        zmq_ctx_set(_ctx, ZMQ_IO_THREADS, io_threads);
    }
    ~ZmqContext() { zmq_ctx_destroy(_ctx); }
    void* get() const noexcept { return _ctx; }

   private:
    void* _ctx{nullptr};
};

// RAII Wrapper for ZeroMQ Socket
struct ZmqSocket {
    ZmqSocket() noexcept = default;
    /**
     * @param ctx      - the raw `void*` from ZmqContext
     * @param type     - one of ZMQ_PUB, ZMQ_SUB, ZMQ_PUSH, ZMQ_PULL, etc.
     * @param addr     - a C++ string_view (must be nullptr-terminated internally before calling)
     * @param bind    - if true → call `zmq_bind()`, else → call `zmq_connect()`.
     */
    ZmqSocket(void* ctx, int type, std::string_view addr, bool bind = false) {
        _socket = zmq_socket(ctx, type);
        assert(_socket);
        if (bind)  // as listener
            zmq_bind(_socket, addr.data());
        else  // as client
            zmq_connect(_socket, addr.data());
    }

    // Delete copy to avoid double‐close
    ZmqSocket(const ZmqSocket&) = delete;
    ZmqSocket& operator=(const ZmqSocket&) = delete;

    // Provide move constructor
    ZmqSocket(ZmqSocket&& other) noexcept
        : _socket(other._socket) {
        other._socket = nullptr;
    }

    // Provide move assignment
    ZmqSocket& operator=(ZmqSocket&& other) noexcept {
        if (this != &other) {
            if (_socket) zmq_close(_socket);
            _socket = other._socket;
            other._socket = nullptr;
        }
        return *this;
    }

    ~ZmqSocket() {
        if (_socket) {
            zmq_close(_socket);
            _socket = nullptr;
        }
    }
    void* get() const noexcept { return _socket; }

   private:
    void* _socket{nullptr};
};

struct CtaEngine {
    /**
     * @param zmq_addr  - path to your zeromq url
     * @param thread_num    - number of worker threads for Taskflow’s Executor
     */
    CtaEngine(std::string_view zmq_addr, size_t thread_num = std::thread::hardware_concurrency())
        : _executor(thread_num),  // worker number
          _context(1),
          _md_sub_socket(_context.get(), ZMQ_SUB, zmq_addr.data(), false) {
        // no limit for recv message numbers, adjust as needed
        int rcvhwm = 0;
        zmq_setsockopt(_md_sub_socket.get(), ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
    }

    /**
     * Add one strategy object for a given symbol. Internally we keep a
     * phmap::flat_hash_map<std::string, vector<CtaStrategy>>, so that
     * we can find all strategies for “AAPL” quickly.
     */
    void addStrategy(std::string_view symbol, CtaStrategy strategy) {
        _stg_map[symbol].emplace_back(std::move(strategy));
        zmq_setsockopt(_md_sub_socket.get(), ZMQ_SUBSCRIBE, symbol.data(), symbol.length());
    }

    /**
     * Start the engine. This spins up two things:
     *   1) A “background thread” which continuously pops Orders from _channel and sends them
     *      out via a PUSH socket.
     *   2) A loop which polls the SUB socket for ticks; on each tick we schedule a small
     *      “processTick” lambda via executor.silent_async(...).
     *
     * Note: once stop() is called, _running flips to false, everything exits cleanly.
     *       The jthread’s destructor will join automatically, and executor.wait_for_all()
     *       waits until all in-flight tasks finish before returning.
     */
    void start() {
        // 1) Launch the “order‐pusher” thread
        _order_push_thread = std::jthread([this] {
            ZmqSocket order_push_socket(_context.get(), ZMQ_PUSH, "ipc://@orders", false);
            while (_running.load(std::memory_order_relaxed)) {
                if (auto order = _channel.pop()) {
                    print_struct(&order.value());
                    zmq_send(order_push_socket.get(), &order.value(), sizeof(Order), 0);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // Avoid busy waiting
                }
            }
        });

        // 2) In the main thread, start polling for ticks
        zmq_pollitem_t items[] = {{_md_sub_socket.get(), 0, ZMQ_POLLIN, 0}};
        while (_running.load(std::memory_order_relaxed)) {
            zmq_poll(items, 1, 100);  // Wait 100ms at most for socket
            TickData tick{};
            if (items[0].revents & ZMQ_POLLIN &&
                zmq_recv(_md_sub_socket.get(), &tick, sizeof(TickData), 0) > 0) {
                // print_struct(&tick);
                _executor.silent_async([this, tick = std::move(tick)] {
                    // look up only once
                    auto it = _stg_map.find(tick.symbol);
                    if (it != _stg_map.end()) {
                        for (auto&& stg : it->second) {
                            // visit strategy with the tick data
                            if (auto order = std::visit([&tick](auto&& strategy) -> std::optional<Order> { return strategy.onTick(tick); }, stg)) {
                                // send to another
                                this->_channel.push(order.value());
                            }
                        }
                    }
                });
            }
        }
        // 3)We’ve exited the poll‐loop because stop() was called or an error occurred.
        // Wait for all in-flight tasks to finish:
        _executor.wait_for_all();

        // if main thread exit due to error, we should also stop the order push thread
        _running.store(false, std::memory_order_relaxed);
    }

    void stop() {
        _running.store(false, std::memory_order_relaxed);
    }

   private:
    ZmqContext _context;  // Single IO thread for simplicity
    ZmqSocket _md_sub_socket;
    tf::Executor _executor;

    lockfree::MPSC<Order, 1024> _channel{};

    phmap::flat_hash_map<std::string, std::vector<CtaStrategy>> _stg_map{};
    std::atomic<bool> _running{true};
    std::jthread _order_push_thread;  // Thread for pushing orders to the order socket
};