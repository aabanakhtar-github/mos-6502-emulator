#ifndef HARTE_TEST_H
#define HARTE_TEST_H

#include <string>
#include "nlohmann/json.hpp"
#include "mos6502.h"
#include <vector> 
#include <tuple> 
#include "types.h"

// don't need all those bytes
struct MemoryState
{
    std::vector<std::pair<std::size_t, Byte>> mem_states;
};

class HarteTest
{
public:
    explicit HarteTest(const std::string& file);
    bool run();
    
    nlohmann::json json;
    Emulator initial_state;
    MemoryState initial_mem_state;

    Emulator final_state;
    MemoryState final_mem_state;

    size_t cycles; 
};

#endif // HARTE_TEST_H