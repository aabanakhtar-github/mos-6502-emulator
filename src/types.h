#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <limits> 

using Byte = std::uint8_t;
using SignedByte = std::int8_t;
using Word = std::uint16_t;

constexpr std::size_t WORD_MAX = std::numeric_limits<Word>::max();
constexpr int CLOCK_uS = 1; // The clock time of the cpu, assuming 1 MHZ model

#endif // TYPES_H