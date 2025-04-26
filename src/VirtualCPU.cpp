#include "VirtualCPU.hpp"
#include "Consts.hpp"
#include <random>
#include <iostream>

VirtualCPU::VirtualCPU() {
    m_regs.fill(0x0);
    m_stack.resize(0x0);
    m_keys.fill(KeyState::NotPressed);

    m_indexRegister = 0x0;
    m_delayTimer = 0x0;
    m_soundTimer = 0x0;

    m_programCounter = PROCESSOR_INTERNAL_PROGRAM_COUNTER;
}

void VirtualCPU::UpdateTimers(AudioPlayer& audioPlayer) {
    if (m_delayTimer > 0) m_delayTimer--;
    if (m_soundTimer > 0) {
        m_soundTimer--;

        /*
         * Maybe too much, try/catch can be very CPU cycles hungry.
         * May have a performance hit, is it noticeable tho ?
        */
        if (m_soundTimer == 1)
            try {
                audioPlayer.PlayBeepSound();
            } catch (std::exception& e) {
                std::cerr << e.what() << "\n";
                std::exit(1);
            }
    }
}

uint16_t VirtualCPU::FetchNextOpcode(Memory& memory) {
    uint16_t msb = memory.Read(m_programCounter); // Most Significant.
    uint16_t lsb = memory.Read(m_programCounter + 1); // Least Significant.

    return msb << 8 | lsb;
}

void VirtualCPU::UnknownOpcode(uint16_t opcode) {
    std::cerr << "[ERROR] Unknown Opcode -> 0x" << std::hex << opcode << std::dec << "\n";
    std::exit(1);
}

void VirtualCPU::ExecuteSingleInstruction(uint16_t opcode, Memory& memory, RenderTable& renderTable) {
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t nn = opcode & 0x00FF;
    uint8_t n = opcode & 0x000F;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;


    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    // CLS
                    renderTable.Clear();
                    m_programCounter += 2;
                    break;
                case 0x00EE:
                    // RET
                    m_programCounter = m_stack.back();
                    m_stack.pop_back();
                    m_programCounter += 2;
                    break;
                default:
                    UnknownOpcode(opcode);
                    break;
            }
            break;
        case 0x1000:
            // JP addr
            m_programCounter = nnn;
            break;
        case 0x2000:
            // CALL addr
            m_stack.push_back(m_programCounter);
            m_programCounter = nnn;
            break;
        case 0x3000:
            // SE Vx, byte
            if (m_regs[x] == nn) m_programCounter += 4;
            else m_programCounter += 2;
            break;
        case 0x4000:
            // SNE Vx, byte
            if (m_regs[x] != nn) m_programCounter += 4;
            else m_programCounter += 2;
            break;
        case 0x5000:
            // SE Vx, Vy
            if (m_regs[x] == m_regs[y]) m_programCounter += 4;
            else m_programCounter += 2;
            break;
        case 0x6000:
            // LD Vx, byte
            m_regs[x] = nn;
            m_programCounter += 2;
            break;
        case 0x7000:
            // ADD Vx, byte
            m_regs[x] += nn;
            m_programCounter += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    // LD Vx, Vy
                    m_regs[x] = m_regs[y];
                    m_programCounter += 2;
                    break;
                case 0x0001:
                    // OR Vx, Vy
                    m_regs[x] |= m_regs[y];
                    m_programCounter += 2;
                    break;
                case 0x0002:
                    // AND Vx, Vy
                    m_regs[x] &= m_regs[y];
                    m_programCounter += 2;
                    break;
                case 0x0003:
                    // XOR Vx, Vy
                    m_regs[x] ^= m_regs[y];
                    m_programCounter += 2;
                    break;
                case 0x0004:
                    // ADD Vx, Vy
                    {
                        uint16_t r = m_regs[x] + m_regs[y];

                        m_regs[0xF] = r > 0xFF ? 1 : 0;

                        m_regs[x] = static_cast<uint8_t>(r & 0xFF);

                        m_programCounter += 2;
                    }
                    break;
                case 0x0005:
                    // SUB Vx, Vy
                    m_regs[0xF] = m_regs[x] > m_regs[y] ? 1 : 0;

                    m_regs[x] -= m_regs[y];

                    m_programCounter += 2;
                    break;
                case 0x0006:
                    // SHR Vx {, Vy}
                    m_regs[0xF] = m_regs[x] & 0x1;

                    m_regs[x] >>= 1;

                    m_programCounter += 2;
                    break;
                case 0x0007:
                    // SUBN Vx, Vy
                    m_regs[0xF] = m_regs[y] > m_regs[x] ? 1 : 0;
                    m_regs[x] = m_regs[y] - m_regs[x];
                    m_programCounter += 2;
                    break;
                case 0x000E:
                    // SHL Vx {, Vy}
                    m_regs[0xF] = (m_regs[x] & 128) >> 7;
                    m_regs[x] <<= 1;
                    m_programCounter += 2;
                    break;
                default:
                    UnknownOpcode(opcode);
                    break;
            }
            break;
        case 0x9000:
            // SNE Vx, Vy
            if (m_regs[x] != m_regs[y]) m_programCounter += 4;
            else m_programCounter += 2;
            break;
        case 0xA000:
            // LD I, addr
            m_indexRegister = nnn;
            m_programCounter += 2;
            break;
        case 0xB000:
            // JP V0, addr
            m_programCounter = nnn + m_regs[0x0];
            break;
        case 0xC000:
            // RND Vx, byte
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<unsigned int> distr(0x0, 0x100);

                m_regs[x] = distr(gen) & nn;
                m_programCounter += 2;
            }
            break;
        case 0xD000:
            // DRW Vx, Vy, nibble
            {
                uint8_t originX = m_regs[x];
                uint8_t originY = m_regs[y];

                m_regs[0xF] = 0;
                for (uint8_t yCoord = 0; yCoord < n; yCoord++) {
                    uint8_t pixel = memory.Read(m_indexRegister + yCoord);
                    for (uint8_t xCoord = 0; xCoord < 8; xCoord++) {
                        if ((pixel & (0x80 >> xCoord)) != 0) {
                            uint8_t pixelX = (originX + xCoord) % CHIP8_SCREEN_WIDTH;
                            uint8_t pixelY = (originY + yCoord) % CHIP8_SCREEN_HEIGHT;

                            // ON -> OFF + Collision, OFF -> ON
                            if (renderTable.IsPixelOn(pixelX, pixelY)) {
                                renderTable.ChangePixelStateTo(pixelX, pixelY, PixelState::Off);
                                m_regs[0xF] = 1;
                            } else {
                                renderTable.ChangePixelStateTo(pixelX, pixelY, PixelState::On);
                            }
                        }
                    }
                }

                m_programCounter += 2;
            }
            break;
        // Keyboard isn't implemented yet, so the SKP instruction isn't useful yet.
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:
                    // SKP Vx
                    if (m_keys[m_regs[x]] == KeyState::Pressed) m_programCounter += 4;
                    else m_programCounter += 2;
                    break;
                case 0x00A1:
                    // SKNP Vx
                    if (m_keys[m_regs[x]] == KeyState::NotPressed) m_programCounter += 4;
                    else m_programCounter += 2;
                    break;
                default:
                    UnknownOpcode(opcode);
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    // LD Vx, DT
                    m_regs[x] = m_delayTimer;
                    m_programCounter += 2;
                    break;
                case 0x000A:
                    // LD Vx, K
                    std::cerr << "[ERROR] LD Vx, K isn't implemented yet.\n";
                    std::exit(1);
                    break;
                case 0x0015:
                    // LD DT, Vx
                    m_delayTimer = m_regs[x];
                    m_programCounter += 2;
                    break;
                case 0x0018:
                    // LD ST, Vx
                    m_soundTimer = m_regs[x];
                    m_programCounter += 2;
                    break;
                case 0x001E:
                    // ADD I, Vx
                    m_indexRegister += m_regs[x];
                    m_programCounter += 2;
                    break;
                case 0x0029:
                    // LD F, Vx
                    m_indexRegister = m_regs[x] * 5;
                    m_programCounter += 2;
                    break;
                case 0x0033:
                    // LD B, Vx
                    {
                        uint8_t regVal = m_regs[x];

                        memory.Write(m_indexRegister, regVal / 100); // Hundreds.
                        memory.Write(m_indexRegister + 1, (regVal % 100) / 10); // Tens.
                        memory.Write(m_indexRegister + 2, regVal % 10); // Units.

                        m_programCounter += 2;
                    }
                    break;
                case 0x0055:
                    // LD [I], Vx
                    for (uint8_t i = 0; i < x + 1; i++) {
                        memory.Write(m_indexRegister + i, m_regs[i]);
                    }

                    m_programCounter += 2;
                    break;
                case 0x0065:
                    // LD Vx, [I]
                    for (uint8_t i = 0; i < x + 1; i++) {
                        m_regs[i] = memory.Read(m_indexRegister + i);
                    }

                    m_programCounter += 2;
                    break;
                default:
                    UnknownOpcode(opcode);
                    break;
            }
            break;
        default:
            UnknownOpcode(opcode);
            break;
    }
}
