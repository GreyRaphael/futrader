#include "mdclient.h"
#include "tdclient.h"

int main(int, char **) {
    // MdClient md_cli{"openctp.toml"};
    // md_cli.Start();
    // md_cli.Subscribe({"MA505", "rb2507"});

    TdClient td_cli{"openctp.toml"};
    td_cli.Start();
    td_cli.ReqSettlementInfo();
    td_cli.QryTradingAccount();
    td_cli.QryInvestorPosition();
    getchar();
}
