#include "Memory.hpp"
#include <iostream>

Memory::Memory() {
    m_memory.fill(0x0);
}

void Memory::Write(uint16_t addr, uint8_t value) {
    // FIXME: Maybe we should error out instead of a simple warning.
    if (addr > TOTAL_MEMORY_SIZE - 1) {
        std::cerr << "[WARNING] Memory Error : WRITE command is out of range at ADDR -> " << unsigned(addr) << "\n";
        return;
    }

    m_memory[addr] = value;
}

uint8_t Memory::Read(uint16_t addr) {
    // FIXME: Maybe we should error out instead of a simple warning.
    if (addr > TOTAL_MEMORY_SIZE - 1) {
        std::cerr << "[WARNING] Memory Error : WRITE command is out of range at ADDR -> " << unsigned(addr) << "\n";
        return 0x0;
    }

    return m_memory[addr];
}