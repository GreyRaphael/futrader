#include <filesystem>
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

    MdClient md_cli{argv[1]};
    md_cli.Start();
    md_cli.Subscribe({"MA505", "rb2507"});

    // TdClient td_cli{argv[1]};
    // td_cli.Start();
    // td_cli.SettlementInfo();
    // td_cli.QryTradingAccount();
    // td_cli.QryInvestorPosition();
    getchar();
}
