#include "catch2/catch_all.hpp"
#include "harte_test.h"
#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include <iostream>

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

void testCase(const std::string &name)
{
    auto cases = getTestCases(TEST_JSON_PATH + name);
    int i = 0;
    for (const auto &test : cases)
    {
        i++;
        if (i > 30) // codespaces are slow
        {
            break;
        }

        std::string name = test["name"];
        SECTION(name.c_str())
        {
            HarteTest testbed(test.dump());
            REQUIRE(testbed.run() == true);
        }
    }
}

TEST_CASE("c9 - CMP - Imediate Mode")
{
    testCase("c9.json");
}

/*
TEST_CASE("09 - ORA - Immediate Mode")
{
    testCase("09.json");
}

TEST_CASE("05 - ORA - ZPG")
{
    testCase("05.json");
}

TEST_CASE("")*/
