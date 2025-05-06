#include <toml++/toml.h>

#include <dylib.hpp>
#include <print>

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "mdclient.h"
#include "tdclient.h"

int main(int, char **) {
    auto config = toml::parse_file("config.toml");
    auto mode = config["default"]["mode"].value_or("openctp");
    std::println("mode={}", mode);
    auto dylib_path = config[mode]["dylib_path"].value_or("");
    auto front_md = config[mode]["front_md"].value_or("");
    auto front_td = config[mode]["front_td"].value_or("");
    auto broker_id = config[mode]["broker_id"].value_or("");
    auto user_id = config[mode]["user_id"].value_or("");
    auto password = config[mode]["password"].value_or("");
    std::println("dylib_path:{}", dylib_path);
    std::println("front_md:{}", front_md);
    std::println("front_td:{}", front_td);
    std::println("broker_id:{}", broker_id);
    std::println("user_id:{}", user_id);
    std::println("password:{}", password);

    dylib lib_md{dylib_path, "thostmduserapi_se.so", dylib::no_filename_decorations};
    dylib lib_td{dylib_path, "thosttraderapi_se.so", dylib::no_filename_decorations};

    auto get_md_ver = lib_md.get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    auto get_td_ver = lib_td.get_function<const char *()>("_ZN19CThostFtdcTraderApi13GetApiVersionEv");
    std::println("md_ver={}", get_md_ver());
    std::println("td_ver={}", get_td_ver());

    auto create_mdapi = lib_md.get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");
    auto create_tdapi = lib_td.get_function<CThostFtdcTraderApi *(const char *)>("_ZN19CThostFtdcTraderApi19CreateFtdcTraderApiEPKc");
    auto mdapi = create_mdapi("", false, false);
    auto tdapi = create_tdapi("");

    MdClient client{mdapi, user_id};
    // TdClient tdcli{tdapi, user_id};

    mdapi->RegisterSpi(&client);
    mdapi->RegisterFront(const_cast<char *>(front_md));
    mdapi->Init();
    mdapi->Join();

    std::println("-----------");
    getchar();
}
