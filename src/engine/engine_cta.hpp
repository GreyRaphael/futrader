#pragma once

#include <parallel_hashmap/phmap.h>

#include <cassert>
#include <filesystem>
#include <string_view>
#include <variant>
#include <vector>

#include "config_parser.h"
#include "i_quote.h"
#include "stra_cta.h"
#include "zmq.h"

struct CtaZmqEngine {
    CtaZmqEngine(std::string_view cfg_filename) {
        assert(std::filesystem::exists(cfg_filename));
        auto config = ZmqConfig::readConfig(cfg_filename);

        // initialize strategies
        for (auto&& symbol : config.symbols) {
            // Create a vector of strategies for each symbol
            _cta_stg_map[symbol] = std::vector<CtaStrategy>{};
            // Add strategies to the vector
            _cta_stg_map[symbol].emplace_back(DualThrust{});
            _cta_stg_map[symbol].emplace_back(Aberration{});
        }

        // Initialize ZeroMQ context and subscriber socket
        _context = zmq_ctx_new();
        _subscriber = zmq_socket(_context, ZMQ_SUB);
        // Set high-water mark for inbound messages
        int rcvhwm = 0;  // Adjust as needed
        zmq_setsockopt(_subscriber, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
        // Connect to IPC address
        zmq_connect(_subscriber, config.address.c_str());
        // Subscribe to topics
        for (auto&& topic : config.symbols) {
            zmq_setsockopt(_subscriber, ZMQ_SUBSCRIBE, topic.data(), topic.length());
        }
    }
    ~CtaZmqEngine() {
        zmq_close(_subscriber);
        zmq_ctx_term(_context);
    }

    void start() {
        TickData tick{};
        while (true) {
            // Receive messages
            int rc = zmq_recv(_subscriber, &tick, sizeof(TickData), 0);
            if (rc == -1) {
                // Handle receive error
                continue;
            }

            for (auto&& stg : _cta_stg_map[tick.symbol]) {
                // visit strategy with the tick data
                std::visit([&tick](auto&& strategy) { auto order = strategy.onTick(tick); }, stg);
            }
        }
    }

   private:
    void* _context{nullptr};
    void* _subscriber{nullptr};
    phmap::flat_hash_map<std::string_view, std::vector<CtaStrategy>> _cta_stg_map;
};
