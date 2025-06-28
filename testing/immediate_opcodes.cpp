#include "catch2/catch_all.hpp"
#include "harte_test.h"
#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include <iostream>

// aint tryna test 9000 cases bruh
constexpr static size_t MAX_TESTS = 9000;
constexpr static auto TEST_JSON_PATH = "../testing/ProcessorTests/6502/v1/";
using namespace nlohmann;

std::vector<json> getTestCases(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    try
    {
        json j = json::parse(buffer.str());
        return j.get<std::vector<json>>();
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        throw;
    }
}

TEST_CASE("6502 Harte Functional Tests - LDA a9 IMMEDIATE")
{
    auto cases = getTestCases(TEST_JSON_PATH + std::string{"a9.json"});

    for (int i = 0; i < cases.size() && i < MAX_TESTS; ++i)
    {
        auto testbed = HarteTest(cases[i].dump()); 
        REQUIRE(testbed.run());
    }
}

TEST_CASE("6502 Harte Functional Tests - ORA 09 IMMEDIATE")
{
    auto cases = getTestCases(TEST_JSON_PATH + std::string{"09.json"});

    for (int i = 0; i < cases.size() && i < MAX_TESTS; ++i)
    {
        auto testbed = HarteTest(cases[i].dump()); 
        REQUIRE(testbed.run());
    }
}
