#include <bt_md.hpp>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>
#include <zmq.hpp>

#include "quote_type.h"
#include "struct_parser.hpp"

int main() {
    std::string_view cfg_filename = "nng.toml";
    assert(std::filesystem::exists(cfg_filename));
    auto config = NngConfig::readConfig(cfg_filename);

    zmq::context_t context{1};  // 1 I/O thread
    zmq::socket_t publisher(context, zmq::socket_type::pub);

    // Set high-water mark for outbound messages
    publisher.set(zmq::sockopt::sndhwm, 0);

    // Bind to the same address (assuming it's something like "ipc:///tmp/your_socket")
    publisher.bind(config.address);

    BactestMdClient bt_cli{config.broker_file, [&publisher](TickData& tick) {
                               zmq::message_t message(sizeof(TickData));
                               std::memcpy(message.data(), &tick, sizeof(TickData));
                               publisher.send(message, zmq::send_flags::none);
                               //    print_struct(&tick);
                           }};

    bt_cli.subscribe(config.symbols);
    bt_cli.start();
}
