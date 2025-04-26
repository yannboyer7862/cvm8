#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <cstdint>
#include <array>

class Memory {
    public:
        Memory();

        void Write(uint16_t addr, uint8_t value);
        uint8_t Read(uint16_t addr);
    private:
        static constexpr uint16_t TOTAL_MEMORY_SIZE = 4096;
        std::array<uint8_t, TOTAL_MEMORY_SIZE> m_memory;
};

#endif