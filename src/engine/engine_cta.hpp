#pragma once

#include <cassert>
#include <filesystem>
#include <string_view>
#include <vector>

#include "config_parser.h"
#include "i_quote.h"
#include "stra_cta.h"
#include "zmq.h"

struct CtaZmqEngine {
    CtaZmqEngine(std::string_view cfg_filename, std::vector<CtaStrategy> & cta_stgs){

    }
};

// struct CtaZmqEngine {
//     CtaZmqEngine(std::string_view cfg_filename, std::vector<CtaStrategy, typename>)
//         : _cta_stgs(&cta_stgs...) {
//         assert(std::filesystem::exists(cfg_filename));
//         auto config = ZmqConfig::readConfig(cfg_filename);

//         // Initialize ZeroMQ context and subscriber socket
//         _context = zmq_ctx_new();
//         _subscriber = zmq_socket(_context, ZMQ_SUB);
//         // Set high-water mark for inbound messages
//         int rcvhwm = 0;  // Adjust as needed
//         zmq_setsockopt(_subscriber, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));
//         // Connect to IPC address
//         zmq_connect(_subscriber, config.address.c_str());
//         // Subscribe to topics
//         for (auto&& topic : config.symbols) {
//             zmq_setsockopt(_subscriber, ZMQ_SUBSCRIBE, topic.data(), topic.length());
//         }
//     }

//     ~CtaZmqEngine() {
//         zmq_close(_subscriber);
//         zmq_ctx_term(_context);
//     }

//     void start() {
//         TickData tick{};
//         while (true) {
//             // Receive messages
//             int rc = zmq_recv(_subscriber, &tick, sizeof(TickData), 0);
//             if (rc == -1) {
//                 // Handle receive error
//                 continue;
//             }

//             // Process tick data with each strategy
//             std::apply([&tick](auto*... stgs) {
//                 ((stgs->onTick(tick)), ...);
//             },
//                        _cta_stgs);
//         }
//     }

//    private:
//     std::vector<CtaStrategy> _cta_stgs;
//     void* _context{nullptr};
//     void* _subscriber{nullptr};
// };