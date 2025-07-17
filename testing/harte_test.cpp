#include "harte_test.h"
#include <fstream>
#include "catch2/catch_all.hpp"
#include <iostream> 


using namespace nlohmann;

HarteTest::HarteTest(const std::string& file)
{
    auto json = json::parse(file);

    // TODO: just use cpu struct instead
    // load up the values
    initial_state.cpu.program_counter = json["initial"]["pc"];
    initial_state.cpu.S = json["initial"]["s"]; 
    initial_state.cpu.accumulator = json["initial"]["a"];
    initial_state.cpu.X = json["initial"]["x"]; 
    initial_state.cpu.Y = json["initial"]["y"];
    initial_state.cpu.P = json["initial"]["p"];

    for (const auto& pair : json["initial"]["ram"]) 
    {
        initial_mem_state.mem_states.push_back(
            std::make_pair((std::size_t)pair[0], (Byte)pair[1])
        );
    }

    // Load up the values for the final state
    final_state.cpu.program_counter = json["final"]["pc"];
    final_state.cpu.S = json["final"]["s"];
    final_state.cpu.accumulator = json["final"]["a"];
    final_state.cpu.X = json["final"]["x"];
    final_state.cpu.Y = json["final"]["y"];
    final_state.cpu.P = json["final"]["p"];

    for (const auto& pair : json["final"]["ram"]) 
    {
        final_mem_state.mem_states.push_back(
            std::make_pair((std::size_t)pair[0], (Byte)pair[1])
        );
    }

    cycles = json["cycles"].size();
}

bool HarteTest::run() 
{
    static int id = 0;
    Emulator testbed;
    testbed.testing = true;
    testbed.cpu = initial_state.cpu;
    if (testbed.cpu.P & MOS_6502::P_DECIMAL) 
    {
        return true;
    }

    for (auto& [addr, val] : initial_mem_state.mem_states) 
    {
        testbed.mem.memory[addr] = val;
    }

    // execute the instruction
    testbed.cycle(); 
        
    REQUIRE((uint32_t)testbed.cpu.accumulator == (uint32_t)final_state.cpu.accumulator);
    REQUIRE((int)testbed.cpu.X == (int)final_state.cpu.X);
    REQUIRE((int)testbed.cpu.Y == (int)final_state.cpu.Y);
    REQUIRE((int)testbed.cpu.program_counter == (int)final_state.cpu.program_counter);
    REQUIRE((int)testbed.cpu.S == (int)final_state.cpu.S);
    REQUIRE((int)testbed.cpu.P == (int)final_state.cpu.P); // Processor status flags
    
    for (auto& [addr, val] : final_mem_state.mem_states) 
    {
        REQUIRE((int)testbed.mem.memory[addr] == (int)val);
    }

    return true;
}