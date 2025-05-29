#include "catch2/catch_all.hpp"
#include "harte_test.h"
#include <string>

constexpr static auto TEST_JSON_PATH = "testing/ProcessorTests/6502/v1/";

TEST_CASE("ORA - OR with accumulator - IMMEDIATE MODE") 
{
    HarteTest test(std::string(TEST_JSON_PATH) + "09.json");
}