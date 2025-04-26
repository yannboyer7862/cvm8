#ifndef VIRTUAL_CPU_HPP
#define VIRTUAL_CPU_HPP

#include "Audio.hpp"
#include "RenderTable.hpp"
#include "Memory.hpp"

#include <vector>
#include <array>
#include <cstdint>

// NOTE: Keyboard not really implemented yet, only placeholder.
enum class KeyState {
    Pressed,
    NotPressed,
};

class VirtualCPU {
    public:
        VirtualCPU();
        
        void UpdateTimers(AudioPlayer& audioPlayer);
        uint16_t FetchNextOpcode(Memory& memory);
        void ExecuteSingleInstruction(uint16_t opcode, Memory& memory, RenderTable& renderTable);
    private:
        void UnknownOpcode(uint16_t opcode);

        static constexpr uint8_t REGISTERS_COUNT = 16; 
        static constexpr uint8_t KEYS_COUNT = 16;

        std::array<uint8_t, REGISTERS_COUNT> m_regs;
        std::vector<uint16_t> m_stack;
        std::array<KeyState, KEYS_COUNT> m_keys;

        uint16_t m_indexRegister;
        uint8_t m_delayTimer;
        uint8_t m_soundTimer;

        uint16_t m_programCounter;
};

#endif
