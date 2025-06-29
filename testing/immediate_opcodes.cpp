#include "catch2/catch_all.hpp"
#include "harte_test.h"
#include <string>
#include <fstream>
#include <vector>
#include "nlohmann/json.hpp"
#include <iostream>

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

void runHarteTest(const std::string& opcodeFile)
{
    auto cases = getTestCases(TEST_JSON_PATH + opcodeFile);

    for (size_t i = 0; i < cases.size() && i < MAX_TESTS; ++i)
    {
        std::cout << "Running case #" << i << " for testfile " << opcodeFile << ":\n"; 
        auto testbed = HarteTest(cases[i].dump());
        REQUIRE(testbed.run());
    }
}

#define LDA_TEST_CASE(NAME, FILE) \
TEST_CASE("6502 Harte Functional Tests - LDA " NAME) { runHarteTest(FILE); }

#if 1 
LDA_TEST_CASE("A9 IMMEDIATE", "a9.json")
LDA_TEST_CASE("A5 ZEROPAGE", "a5.json")
LDA_TEST_CASE("B5 ZEROPAGE,X", "b5.json")
LDA_TEST_CASE("AD ABSOLUTE", "ad.json")
LDA_TEST_CASE("BD ABSOLUTE,X", "bd.json")
LDA_TEST_CASE("B9 ABSOLUTE,Y", "b9.json")
LDA_TEST_CASE("A1 (INDIRECT,X)", "a1.json")
LDA_TEST_CASE("B1 (INDIRECT),Y", "b1.json")
#else 
LDA_TEST_CASE("Testbed: ", "testbed.json")
#endif