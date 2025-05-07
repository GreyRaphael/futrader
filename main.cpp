#include "mdclient.h"
#include "tdclient.h"

int main(int, char **) {
    // MdClient md_cli{};
    // md_cli.Start();
    // md_cli.Subscribe({"MA509", "rb2509"});

    TdClient td_cli{};
    td_cli.Start();
    td_cli.ReqSettlementInfo();
    td_cli.QryInvestorPosition();
    getchar();
}
