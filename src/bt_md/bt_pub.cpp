#include <nng/nng.h>
#include <nng/protocol/pubsub0/pub.h>

#include <bt_md.hpp>

#include "quote_type.h"

int main() {
    std::string_view cfg_filename = "nng.toml";
    assert(std::filesystem::exists(cfg_filename));
    auto config = NngConfig::readConfig(cfg_filename);

    nng_socket pub_sock{};
    nng_pub0_open(&pub_sock);
    nng_listen(pub_sock, config.address.data(), nullptr, 0);  // listener=NULL; flags=0 ignored

    BactestMdClient bt_cli{config.broker_file, [&pub_sock](TickData& tick) {
                               // nng_msleep(1);
                               nng_send(pub_sock, &tick, sizeof(TickData), 0);  // flags=0, default for pub mode
                           }};
    bt_cli.subscribe(config.symbols);
    bt_cli.start();
}