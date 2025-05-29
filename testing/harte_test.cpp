#include "harte_test.h"
#include <fstream>

using namespace nlohmann;

HarteTest::HarteTest(const std::string& file)
{
    std::ifstream f(file);
    if (!f.is_open()) { return; }
    
    std::stringstream buf; 
    buf << f.rdbuf(); 
    std::string as_string = buf.str(); 

    json = json::parse(as_string);
    f.close();

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
}

bool HarteTest::run() 
{
    Emulator testbed; 
    testbed.cpu = initial_state.cpu;
    for (auto& [addr, val] : initial_mem_state.mem_states) 
    {
        testbed.mem.memory[addr] = val;
    }

    testbed.run(); 

    if (testbed.cpu != final_state.cpu) 
    {
        return false; 
    }

    for (auto& [addr, val] : final_mem_state.mem_states) 
    {
        if (testbed.mem.memory[addr] != val) 
        {
            return false;
        }
    }

    return true;
}