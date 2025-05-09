#include "6502.h"
/**
 * This is a simple 6502 emulator in C++.
 * It implements the CPU, memory, and a simple test program.
 * Based on https://www.youtube.com/watch?v=qJgsuQoy9bc
 */

/**
 * The main function in which the emulator runs on.
 */
int main() {
    Memory memory;
    CPU cpu;
    cpu.reset(memory);

    // - Start Program -
    memory[0xFFFC] = CPU::INS_JSR;
    memory[0xFFFD] = 0x42;
    memory[0xFFFE] = 0x42;
    memory[0x4242] = CPU::INS_LDA_IM;
    memory[0x4243] = 0x84;
    // - End Program -

    cpu.execute(memory, 9);
    return EXIT_SUCCESS;
}