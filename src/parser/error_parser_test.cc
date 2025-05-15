#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "error_parser.hpp"

#include <doctest/doctest.h>

#include <print>

TEST_CASE("testing error msg") {
    std::println("{}", errconfig::get(0));
    std::println("{}", errconfig::get(15));
    std::println("{}", errconfig::get(38));
}

TEST_CASE("testing disconnection errors") {
    std::println("{}", errconfig::discon_errors.at(0x1001));
    std::println("{}", errconfig::discon_errors.at(0x1002));
    std::println("{}", errconfig::discon_errors.at(0x2001));
    std::println("{}", errconfig::discon_errors.at(0x2002));
    std::println("{}", errconfig::discon_errors.at(0x2003));
}

typedef char TThostFtdcErrorMsgType[81];

/// 响应信息
struct CThostFtdcRspInfoField {
    /// 错误代码
    int ErrorID;
    /// 错误信息
    TThostFtdcErrorMsgType ErrorMsg;
};

struct MyStruct {
    int id;
    double score;
    std::string name;
};

TEST_CASE("testing disconnection errors") {
    CThostFtdcRspInfoField err_obj{0, "hello"};
    MyStruct obj{100, 99.5, "Tom"};
    handle_resp(&obj, &err_obj);
}