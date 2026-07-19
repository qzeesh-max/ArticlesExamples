#define USE_CONSTEXPR_FOR_HASH
#include <iostream>
#include "fix_enums.h"
#include "near_perfect_hash.hpp"

enum class OrderFieldsEnum
{
    Error = 0,
    ClOrdID = 11,
    OrigClOrdID = 41,
    Price = 44,
    OrdType = 40,
    ExecTransType = 20,
    OrdStatus = 39,
    ExecType = 151,
    LeavesQty = 150,
    CumQty = 14,
    OrdQty = 38,
    LastPx = 31,
    LastShares = 32,
    OrderID = 37,
    ExecID = 17,
    RefExecID = 19,
    AvgPx = 6,
    Symbol = 55,
    ExecBroker = 76,
    TransactTime = 60,
    Account = 1,
    AccountType = 581,
    Side = 54,
    TimeInForce = 59,
    Currency = 15,
    StopPx = 99,
    LocateReqd = 114,
    ShortMarkingExemptIndicator = 1688,
    MinQty = 110,
    MinQtyMethod = 1822,
    SettlType = 63,
    SettlDate = 64,
    ExpireTime = 126,
    Text = 58
};



CONSTEXPR_OR_CONST_FOR_HASH NearPerfectHash<OrderFieldsEnum, 33> Hash (
    make_enum_name(OrderFieldsEnum::ClOrdID, "ClOrdID"),
    make_enum_name(OrderFieldsEnum::OrigClOrdID, "OrigClOrdID"),
    make_enum_name(OrderFieldsEnum::Price, "Price"),
    make_enum_name(OrderFieldsEnum::OrdType, "OrdType"),
    make_enum_name(OrderFieldsEnum::ExecTransType, "ExecTransType"),
    make_enum_name(OrderFieldsEnum::OrdStatus, "OrdStatus"),
    make_enum_name(OrderFieldsEnum::ExecType, "ExecType"),
    make_enum_name(OrderFieldsEnum::LeavesQty, "LeavesQty"),
    make_enum_name(OrderFieldsEnum::CumQty, "CumQty"),
    make_enum_name(OrderFieldsEnum::OrdQty, "OrdQty"),
    make_enum_name(OrderFieldsEnum::LastPx, "LastPx"),
    make_enum_name(OrderFieldsEnum::LastShares, "LastShares"),
    make_enum_name(OrderFieldsEnum::OrderID, "OrderID"),
    make_enum_name(OrderFieldsEnum::ExecID, "ExecID"),
    make_enum_name(OrderFieldsEnum::RefExecID, "RefExecID"),
    make_enum_name(OrderFieldsEnum::AvgPx, "AvgPx"),
    make_enum_name(OrderFieldsEnum::Symbol, "Symbol"),
    make_enum_name(OrderFieldsEnum::ExecBroker, "ExecBroker"),
    make_enum_name(OrderFieldsEnum::TransactTime, "TransactTime"),
    make_enum_name(OrderFieldsEnum::Account, "Account"),
    make_enum_name(OrderFieldsEnum::AccountType, "AccountType"),
    make_enum_name(OrderFieldsEnum::Side, "Side"),
    make_enum_name(OrderFieldsEnum::TimeInForce, "TimeInForce"),
    make_enum_name(OrderFieldsEnum::Currency, "Currency"),
    make_enum_name(OrderFieldsEnum::StopPx, "StopPx"),
    make_enum_name(OrderFieldsEnum::LocateReqd, "LocateReqd"),
    make_enum_name(OrderFieldsEnum::ShortMarkingExemptIndicator, "ShortMarkingExemptIndicator"),
    make_enum_name(OrderFieldsEnum::MinQty, "MinQty"),
    make_enum_name(OrderFieldsEnum::MinQtyMethod, "MinQtyMethod"),
    make_enum_name(OrderFieldsEnum::SettlType, "SettlType"),
    make_enum_name(OrderFieldsEnum::SettlDate, "SettlDate"),
    make_enum_name(OrderFieldsEnum::ExpireTime, "ExpireTime"),
    make_enum_name(OrderFieldsEnum::Text, "Text")
);


int main(void)
{
    std::cout << "size of hash footprint: " << sizeof(Hash) << std::endl;
    std::cout << "collisions: " << Hash.GetCollisions() << std::endl;

    std::cout << "Verifying Hash:" << std::endl;

    auto countErrors = 0;

    for (const auto& ax : Hash.GetValues())
    {
        if (Hash.Convert(ax.enumName_,OrderFieldsEnum::Error) != ax.enumVal_)
        {
            std::cout << "Error : " << ax.enumName_ << " not matching expectation of enumeration." << std::endl;
            ++countErrors;
        }
    }

    std::cout << "Verification finished with " << countErrors << " error(s)" << std::endl;


    std::cout << std::endl << std::endl;

    std::cout << "collisions in full fix tags: " << FixTagHash.GetCollisions() << std::endl;
    std::cout << "re-used buckets in fix tags: "<< FixTagHash.GetReusedBuckets() << std::endl;
    std::cout << "Test Lookup Account :" << static_cast<int>(FixTagHash.Convert("Account", static_cast<FixTags>(0))) << std::endl;
    std::cout << "Test Lookup Text : " << static_cast<int>(FixTagHash.Convert("Text", static_cast<FixTags>(0))) << std::endl;

    return 0;
}
