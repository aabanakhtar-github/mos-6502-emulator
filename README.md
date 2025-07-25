# MOS 6502 Emulator

An educational C++ emulator for the classic [MOS Technology 6502](https://en.wikipedia.org/wiki/MOS_Technology_6502) 8-bit processor, built from the ground up to closely mirror real hardware behavior.

[Demo here](https://www.youtube.com/watch?v=HIYumfzdDtw)
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
| Stack Range | 0x0100–0x01FF | Fixed 256-byte hardware stack                 |

## Building & Running

You will need CMake and a C++ compiler to build this.

```bash
$ mkdir build
$ cd build 
$ cmake -S .. 
$ cmake --build . 
$ ./mos_6502_emulator
```

You will be asked to enter a binary file. To create a 6502 binary I recommend using ```ca65```. A simple executable with a makefile is located in ```testing/asm```. To build it, run the makefile.

## Output

Running the test assembly file:

```mipsasm
.segment "CODE"          ; Tell linker this is the CODE segment
.org $8000

RESET:  SEI
        CLD
        CLC
        LDA #$05
        ADC #$03
        BRK
```

Results in:

```bash
Enter binary file: basic_file.bin
=====INITIAL=====
PC: $8000  A: $00  X: $00  Y: $00  S: $FD  P: $20
Loaded ROM successfully.
====FINAL=====
PC: $8007  A: $08  X: $00  Y: $00  S: $FD  P: $24
```

## Tests

You can run the Catch2 Tests with ```ctest```. Please note that decimal mode is unimplemented (and disabled in the test suite) and that the Harte Tests have an invalid test case (#4045) for JSR(20 ABSOLUTE).

## TODO

- Decimal Mode
