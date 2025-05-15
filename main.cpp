#include <filesystem>
#include <memory>
#include <print>

#include "mdclient.h"
#include "tdclient.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println("Usage: {} xxx.toml", argv[0]);
        return 1;
    }
    if (!std::filesystem::exists(argv[1])) {
        std::println("Error: {} not exist!", argv[1]);
        return 2;
    }

    auto channel_ptr = std::make_shared<MarketDataChannel>();

    MdClient md_cli{argv[1], channel_ptr};
    md_cli.Start();
    md_cli.Subscribe({"MA505", "rb2507"});

    while (true) {
        auto value = channel_ptr->pop();
        if (!value) {
            std::println("empty, cannot pop");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        // print_struct(&value);
        std::println("{},{},{}", value->UpdateTime, value->InstrumentID, value->LastPrice);
    }

    // TdClient td_cli{argv[1]};
    // td_cli.Start();
    // S Z D J N
    // td_cli.QryInstrument({"SHFE", "INE", "CZCE"});
    // td_cli.QryInstrument({});
    // td_cli.QryExchange();
    // td_cli.QryProduct();
    // td_cli.QryInstrumentCommissionRate();
    // td_cli.QryInstrumentOrderCommRate();
    // td_cli.SettlementInfo();
    // td_cli.QryTradingAccount();
    // td_cli.QryInvestorPosition();
    getchar();
}
