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

void runHarteTest(const std::string& opcodeFile)
{
    auto cases = getTestCases(TEST_JSON_PATH + opcodeFile);

    for (size_t i = 0; i < cases.size(); ++i)
    {
        std::cout << "Running case #" << i << " for testfile " << opcodeFile << ":\n"; 
        auto testbed = HarteTest(cases[i].dump());
        REQUIRE(testbed.run());
    }
}

#define _TEST_CASE(NAME, FILE) \
SECTION("6502 Harte Functional Tests - " NAME) { runHarteTest(FILE); }
/*
// 6502 Emulator Instruction Tests
TEST_CASE("BRK") {
    _TEST_CASE("00 IMPLICIT", "00.json")
}

TEST_CASE("LDA") {
    _TEST_CASE("A9 IMMEDIATE", "a9.json")
    _TEST_CASE("A5 ZEROPAGE", "a5.json")
    _TEST_CASE("B5 ZEROPAGE,X", "b5.json")
    _TEST_CASE("AD ABSOLUTE", "ad.json")
    _TEST_CASE("BD ABSOLUTE,X", "bd.json")
    _TEST_CASE("B9 ABSOLUTE,Y", "b9.json")
    _TEST_CASE("A1 (INDIRECT,X)", "a1.json")
    _TEST_CASE("B1 (INDIRECT),Y", "b1.json")
}

TEST_CASE("LDX") {
    _TEST_CASE("A2 IMMEDIATE", "a2.json")
    _TEST_CASE("A6 ZEROPAGE", "a6.json")
    _TEST_CASE("B6 ZEROPAGE,Y", "b6.json")
    _TEST_CASE("AE ABSOLUTE", "ae.json")
    _TEST_CASE("BE ABSOLUTE,Y", "be.json")
}

TEST_CASE("LDY") {
    _TEST_CASE("A0 IMMEDIATE", "a0.json")
    _TEST_CASE("A4 ZEROPAGE", "a4.json")
    _TEST_CASE("B4 ZEROPAGE,X", "b4.json")
    _TEST_CASE("AC ABSOLUTE", "ac.json")
    _TEST_CASE("BC ABSOLUTE,X", "bc.json")
}

TEST_CASE("STA") {
    _TEST_CASE("85 ZEROPAGE", "85.json")
    _TEST_CASE("95 ZEROPAGE,X", "95.json")
    _TEST_CASE("8D ABSOLUTE", "8d.json")
    _TEST_CASE("9D ABSOLUTE,X", "9d.json")
    _TEST_CASE("99 ABSOLUTE,Y", "99.json")
    _TEST_CASE("81 (INDIRECT,X)", "81.json")
    _TEST_CASE("91 (INDIRECT),Y", "91.json")
}

TEST_CASE("ORA") {
    _TEST_CASE("09 IMMEDIATE", "09.json")
    _TEST_CASE("05 ZEROPAGE", "05.json")
    _TEST_CASE("15 ZEROPAGE,X", "15.json")
    _TEST_CASE("0D ABSOLUTE", "0d.json")
    _TEST_CASE("1D ABSOLUTE,X", "1d.json")
    _TEST_CASE("19 ABSOLUTE,Y", "19.json")
    _TEST_CASE("01 (INDIRECT,X)", "01.json")
    _TEST_CASE("11 (INDIRECT),Y", "11.json")
}



TEST_CASE("CMP") {
    _TEST_CASE("C9 IMMEDIATE", "c9.json")
    _TEST_CASE("C5 ZEROPAGE", "c5.json")
    _TEST_CASE("D5 ZEROPAGE,X", "d5.json")
    _TEST_CASE("CD ABSOLUTE", "cd.json")
    _TEST_CASE("DD ABSOLUTE,X", "dd.json")
    _TEST_CASE("D9 ABSOLUTE,Y", "d9.json")
    _TEST_CASE("C1 (INDIRECT,X)", "c1.json")
    _TEST_CASE("D1 (INDIRECT),Y", "d1.json")
}

TEST_CASE("CPX") {
    _TEST_CASE("E0 IMMEDIATE", "e0.json")
    _TEST_CASE("E4 ZEROPAGE", "e4.json")
    _TEST_CASE("EC ABSOLUTE", "ec.json")
}

TEST_CASE("CPY") {
    _TEST_CASE("C0 IMMEDIATE", "c0.json")
    _TEST_CASE("C4 ZEROPAGE", "c4.json")
    _TEST_CASE("CC ABSOLUTE", "cc.json")
}

TEST_CASE("EOR") {
    _TEST_CASE("49 IMMEDIATE", "49.json")
    _TEST_CASE("45 ZEROPAGE", "45.json")
    _TEST_CASE("55 ZEROPAGE,X", "55.json")
    _TEST_CASE("4D ABSOLUTE", "4d.json")
    _TEST_CASE("5D ABSOLUTE,X", "5d.json")
    _TEST_CASE("59 ABSOLUTE,Y", "59.json")
    _TEST_CASE("41 (INDIRECT,X)", "41.json")
    _TEST_CASE("51 (INDIRECT),Y", "51.json")
}

TEST_CASE("ADC") {
    _TEST_CASE("69 IMMEDIATE", "69.json")
    _TEST_CASE("65 ZEROPAGE", "65.json")
    _TEST_CASE("75 ZEROPAGE,X", "75.json")
    _TEST_CASE("6D ABSOLUTE", "6d.json")
    _TEST_CASE("7D ABSOLUTE,X", "7d.json")
    _TEST_CASE("79 ABSOLUTE,Y", "79.json")
    _TEST_CASE("61 (INDIRECT,X)", "61.json")
    _TEST_CASE("71 (INDIRECT),Y", "71.json")
}
*/
TEST_CASE("SBC") {
    _TEST_CASE("E9 IMMEDIATE", "e9.json")
    _TEST_CASE("E5 ZEROPAGE", "e5.json")
    _TEST_CASE("F5 ZEROPAGE,X", "f5.json")
    _TEST_CASE("ED ABSOLUTE", "ed.json")
    _TEST_CASE("FD ABSOLUTE,X", "fd.json")
    _TEST_CASE("F9 ABSOLUTE,Y", "f9.json")
    _TEST_CASE("E1 (INDIRECT,X)", "e1.json")
    _TEST_CASE("F1 (INDIRECT),Y", "f1.json")
}

/*
TEST_CASE("TAX") {
    _TEST_CASE("AA IMPLICIT", "aa.json")
}

TEST_CASE("ASL") {
    _TEST_CASE("0A ACCUMULATOR", "0a.json")
    _TEST_CASE("06 ZEROPAGE", "06.json")
    _TEST_CASE("16 ZEROPAGE,X", "16.json")
    _TEST_CASE("0E ABSOLUTE", "0e.json")
    _TEST_CASE("1E ABSOLUTE,X", "1e.json")
}

TEST_CASE("ROR") {
    _TEST_CASE("6A ACCUMULATOR", "6a.json")
    _TEST_CASE("66 ZEROPAGE", "66.json")
    _TEST_CASE("76 ZEROPAGE,X", "76.json")
    _TEST_CASE("6E ABSOLUTE", "6e.json")
    _TEST_CASE("7E ABSOLUTE,X", "7e.json")
}*/