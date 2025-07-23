# MOS 6502 Emulator

An educational C++ emulator for the classic [MOS Technology 6502](https://en.wikipedia.org/wiki/MOS_Technology_6502) 8-bit processor, built from the ground up to closely mirror real hardware behavior.

## Features

* Full 6502 CPU functionality support (minus decimal mode)
* Accurately implemented register level functionality
* Support for all 151 official instructions
* Unit Tested using the Tom Harte ProcessorTests and Catch2
  * NOTE: There is one test case in the ProcessorTests repo that is invalid.
* Written in readable c++

## Implementation Overview

| Component   | Size           | Description                                   |
| ----------- | -------------- | --------------------------------------------- |
| `A`         | 8-bit          | Accumulator (arithmetic, logic, loads/stores) |
| `X`, `Y`    | 8-bit          | Index registers (looping, memory access)      |
| `S`         | 8-bit          | Stack Pointer (offset from 0x0100–0x01FF)    |
| `PC`        | 16-bit         | Program Counter (tracks instruction location) |
| `P`         | 8-bit          | Processor Status Flags                        |
| Memory      | 64 KB          | Unified address space (RAM + ROM)             |
| Stack Range | 0x0100–0x01FF | Fixed 256-byte hardware stack                 

## Building & Running

Coming soon..

## Tests

Coming soon.

## TODO

Coming soon.

