#include "VM.hpp"
#include "Consts.hpp"
#include <fstream>
#include <vector>

void VM::InitVMAudio() {
    m_audioPlayer.Init();
}

void VM::LoadROMFromFile(const std::string& romPath) {
    std::ifstream romFile(romPath);
    
    if (!romFile.is_open())
        throw std::runtime_error("[ERROR] No program is provided !");
    
    romFile.seekg(0, std::ios::end);
    std::size_t romBufferLen = romFile.tellg();
    romFile.seekg(0, std::ios::beg);

    std::vector<uint8_t> romBuffer;

    romBuffer.reserve(romBufferLen);
    romFile.read(reinterpret_cast<char*>(romBuffer.data()), romBufferLen);
    romFile.close();


    // Load FONTSET before anything else.
    for (std::size_t i = 0; i < FONTSET_SIZE; i++) {
        m_memory.Write(i, FONTSET[i]);
    }

    // We start writing the ROM data after 0x200(where the Program Counter starts).
    for (std::size_t idx = 0; idx < romBufferLen; idx++) {
        m_memory.Write(idx + PROCESSOR_INTERNAL_PROGRAM_COUNTER, romBuffer[idx]);
    }
}

bool VM::IsPixelOn_FromRT(uint8_t x, uint8_t y) {
    return m_renderTable.IsPixelOn(x, y);
}

void VM::ExecuteSingleVirtualCPUInstruction() {
    uint16_t opcode = m_virtualCPU.FetchNextOpcode(m_memory);
    m_virtualCPU.ExecuteSingleInstruction(opcode, m_memory, m_renderTable);
}

void VM::UpdateVirtualCPUTimers() {
    m_virtualCPU.UpdateTimers(m_audioPlayer);
}
