#include "catch2/catch_all.hpp"
#include "harte_test.h"
#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include <iostream>

constexpr static auto TEST_JSON_PATH = "../testing/ProcessorTests/6502/v1/";
using namespace nlohmann;

std::vector<json> getTestCases(const std::string& filename)
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
    catch (const json::parse_error& e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        throw;
    }
}

TEST_CASE("LDA - Load into accumulator") 
{
    auto cases = getTestCases(TEST_JSON_PATH + std::string("apple.json"));
    for (const auto& test : cases)
    {
        std::string name = test["name"];
        SECTION(name.c_str())
        {
            HarteTest testbed(test.dump());
            REQUIRE(testbed.run() == true);
            return;
        }
    }
}