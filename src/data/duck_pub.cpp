#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include <cassert>
#include <filesystem>
#include <string_view>
#include <thread>

#include "config_parser.h"
#include "duck_loader.h"
#include "QuoteType.h"
#include "struct_parser.hpp"

int main(int argc, char const* argv[]) {
    std::string_view cfg_filename = "nng.toml";
    assert(std::filesystem::exists(cfg_filename));
    auto config = NngConfig::read_config(cfg_filename);

    nng_socket pub_sock{};
    nng_pub0_open(&pub_sock);
    nng_listen(pub_sock, config.Address.data(), nullptr, 0);  // listener=NULL; flags=0 ignored

    auto channel_ptr = std::make_shared<TickDataChannel>();

    std::jthread loader{[channel_ptr, &config] {
        HistoryTickLoader loader{config.BrokerFile, channel_ptr};
        loader.Subscribe(config.Symbols);
        loader.Run();
    }};

    while (true) {
        auto value = channel_ptr->pop();

        if (!value) {
            // sleep time in ms
            nng_msleep(config.PollIntervalMs);
            continue;
        }
        print_struct(&value.value());
        nng_send(pub_sock, &value.value(), sizeof(TickData), 0);  // flags=0, default for pub mode
    }
}