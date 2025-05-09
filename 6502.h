#include <stdio.h>
#include <stdlib.h>

/**
 * This is a simple 6502 emulator in C++.
 * It implements the CPU, memory, and a simple test program.
 * Based on https://www.youtube.com/watch?v=qJgsuQoy9bc
 */

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

// The Memory struct the CPU accesses.
struct Memory {

    static constexpr u32 MAX_MEM = 64 * 1024; // The memory size of 64 Kilobytes.
    Byte data[MAX_MEM]; // 1D array of memory.

    // Initializes all the bytes in the memory to 0.
    void init() {
        for (u32 i = 0; i < MAX_MEM; i++) {data[i] = 0;}
    }

    // Read a byte in the memory from a given address.
    Byte operator[](u32 address) const {return data[address];}

    // Write a byte in the memory at a given address.
    Byte& operator[](u32 address) {return data[address];}

    // Write 2 bytes in the memory at a given address. Decrements cycles by 2.
    void write_word(Word word, u32 address, u32& cycles) {
        data[address] = word & 0xFF;
        cycles--;
        data[address + 1] = (word >> 8);
        cycles--;
    }

};

// The 6502 CPU to be emulated. Little endian.
struct CPU {

    Word PC; // Program Counter
    Word SP; // Stack Pointer

    Byte A, X, Y; // Registers

    Byte C : 1; // Carry Flag
    Byte Z : 1; // Zero Flag
    Byte I : 1; // Interrupt Disable
    Byte D : 1; // Decimal Mode Flag
    Byte B : 1; // Break Command
    Byte V : 1; // Overflow Flag
    Byte N : 1; // Status Flags

    // Resets the program counter, stack pointer, registers, status flags and memory.
    void reset(Memory& memory) {
        PC = 0xFFFC;
        SP = 0x0100; // stack access starts at 100
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.init();
    }

    // Retrieves the byte, decrements clock cycle and increments the program counter.
    Byte fetch_byte(Memory& memory, u32& cycles) {
        Byte byte = memory[PC++];
        cycles--;
        return byte;
    }

    // Retrieves the word in *little endian*, decrementing clock cycle by 2.
    // Also increments the program counter.
    Word fetch_word(Memory& memory, u32& cycles) {
        Word word = memory[PC]; // retrieves the first byte
        PC++; cycles--;
        word |= (memory[PC] << 8); // appends the second byte shifted over
        PC++; cycles--;
        return word;
    }
    // Reads the byte at the program counter and decrements the clock cycle.
    Byte read_byte(Memory memory, u32 address, u32& cycles) {
        Byte byte = memory[address];
        cycles--;
        return byte; 
    }

    static constexpr Byte INS_LDA_IM = 0xA9; // Load Immediate instruction. 3 Cycles.
    static constexpr Byte INS_LDA_ZP = 0xA5; // Zero Page instruction.
    static constexpr Byte INS_LDA_ZPX = 0xB5; // Zero Page X intruction.
    static constexpr Byte INS_JSR = 0x20; // Jump to Subroutine instruction. 6 Cycles.

    void LDA_set_status() {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;
    }

    // Executes and handles an instruction with a given amount of cycles.
    void execute(Memory& memory, u32 cycles) {
        while (cycles > 0) {
            printf("cycles = %d\n", cycles);
            Byte instruction = fetch_byte(memory, cycles); // 1
            switch (instruction) {
                case INS_LDA_IM: {
                    Byte value = fetch_byte(memory, cycles); // 1
                    A = value;
                    LDA_set_status();
                    printf("A = %02x\n", A);
                } break;
                case INS_LDA_ZP: {
                    Byte zero_page_address = fetch_byte(memory, cycles);
                    A = read_byte(memory, zero_page_address,cycles);
                    LDA_set_status();
                    printf("A = 0x%02X\n", A);
                } break;
                case INS_LDA_ZPX: {
                    Byte zero_page_address = fetch_byte(memory, cycles);
                    zero_page_address += X; cycles--;
                    A = read_byte(memory, zero_page_address,cycles);
                    LDA_set_status();
                } break;
                case INS_JSR: {
                    Word sub_address = fetch_word(memory, cycles); // 2
                    memory.write_word(PC - 1, SP, cycles); // 2
                    PC = sub_address; cycles--; // 1
                    SP++; cycles--;
                    printf("JSR: PC = %02X\n", PC);
                } break;
                default: {
                    printf("Instruction not handled %d\n", instruction);
                } break;
            }
        }
    }
};
