#include <zmq.h>

#include <bt_md.hpp>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <string>

#include "quote_type.h"

int main() {
    std::string_view cfg_filename = "nng.toml";
    assert(std::filesystem::exists(cfg_filename));
    auto config = NngConfig::readConfig(cfg_filename);

    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);

    // Set high-water mark for outbound messages
    int sndhwm = 0;  // Adjust as needed
    zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

    // Bind to IPC address (e.g., "ipc:///tmp/your_socket")
    zmq_bind(publisher, config.address.c_str());

    BactestMdClient bt_cli{config.broker_file, [publisher](TickData& tick) {
                               zmq_send(publisher, &tick, sizeof(TickData), 0);
                           }};

    bt_cli.subscribe(config.symbols);
    bt_cli.start();

    // Cleanup (will not be reached if bt_cli.start() blocks indefinitely)
    zmq_close(publisher);
    zmq_ctx_term(context);
}
