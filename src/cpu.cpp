#include "cpu.hpp"

#include "isa.hpp"

CPU::CPU(Bus& b) 
    : bus(b) {
    reset();
}

void CPU::reset() {
    x.fill(0);
    pc = 0;
    instr_count = 0;
    in_interrupt = false;
    saved_pc = 0;
    std::cout << "CPU resetada. \n";
}


void CPU::dump() const {
    std::cout << "[CPU] Estado atual:\n";
    std::cout << "PC = " << pc << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << i << " = " << x[i] << "\t";
        if ((i + 1) % 8 == 0) {
            std::cout << "\n";
        }
    }
}

uint32_t CPU::getBits(uint32_t value, int hi, int lo) const {
    uint32_t mask = ((1u << (hi - lo + 1)) - 1u);
    return (value >> lo) & mask;
}

int32_t CPU::signExtend(uint32_t value, int bits) const{
    int32_t s = static_cast<int32_t>(value);
    int shift = 32 - bits;
    return (s << shift) >> shift;
}

void CPU::step(){
    std::cout << "[CPU] Executando ciclo no @PC = " << pc << std::endl;

    uint32_t inst = bus.read32(pc);
    std::cout << "[CPU] Instrucao lida: 0x" << std::hex << inst << std::dec << std::endl;

    uint32_t opcode = getBits(inst, 6, 0);
    uint32_t rd     = getBits(inst, 11, 7);
    uint32_t funct3 = getBits(inst, 14, 12);
    uint32_t rs1    = getBits(inst, 19, 15);
    uint32_t rs2    = getBits(inst, 24, 20);
    uint32_t funct7 = getBits(inst, 31, 25);


    std::cout << "[CPU] opcode= " << opcode
              << ", rd=" << rd
              << ", funct3=" << funct3
              << ", rs1=" << rs1
              << ", rs2=" << rs2
              << ", funct7=" << funct7 << std::endl;

    uint32_t next_pc = pc + 4;

    switch (opcode) {
        case rv32i::OPCODE_LUI: {
            int32_t imm_u = static_cast<int32_t>(inst & 0xFFFFF000);
            if (rd != 0) {
                x[rd] = static_cast<uint32_t>(imm_u);
            }
            std::cout << "[CPU] Executando LUI: x" << rd << " = " << imm_u << std::endl;
            break;
        }
        case rv32i::OPCODE_AUIPC: {
            int32_t imm_u = static_cast<int32_t>(inst & 0xFFFFF000);
            if (rd != 0) {
                x[rd] = pc + imm_u;
            }
            std::cout << "[CPU] Executando AUIPC: x" << rd << " = PC(" << pc
                      << ") + " << imm_u << std::endl;
            break;
        }
        case rv32i::OPCODE_JAL: {
            uint32_t imm20 = getBits(inst, 31, 31);
            uint32_t imm10_1 = getBits(inst, 30, 21);
            uint32_t imm11 = getBits(inst, 20, 20);
            uint32_t imm19_12 = getBits(inst, 19, 12);
            int32_t imm = signExtend((imm20 << 20) | (imm19_12 << 12) |
                                         (imm11 << 11) | (imm10_1 << 1),
                                     21);
            if (rd != 0) {
                x[rd] = pc + 4;
            }
            next_pc = pc + imm;
            std::cout << "[CPU] Executando JAL: x" << rd << " = PC+4, salto para "
                      << next_pc << std::endl;
            break;
        }
        case rv32i::OPCODE_JALR: {
            int32_t imm = signExtend(getBits(inst, 31, 20), 12);
            uint32_t target = (x[rs1] + imm) & ~1u;
            if (rd != 0) {
                x[rd] = pc + 4;
            }
            next_pc = target;
            std::cout << "[CPU] Executando JALR: x" << rd << " = PC+4, salto para "
                      << target << std::endl;
            break;
        }
        case rv32i::OPCODE_BRANCH: {
            uint32_t imm12 = getBits(inst, 31, 31);
            uint32_t imm10_5 = getBits(inst, 30, 25);
            uint32_t imm4_1 = getBits(inst, 11, 8);
            uint32_t imm11 = getBits(inst, 7, 7);
            int32_t imm = signExtend((imm12 << 12) | (imm11 << 11) |
                                         (imm10_5 << 5) | (imm4_1 << 1),
                                     13);

            bool take = false;
            switch (funct3) {
                case rv32i::FUNCT3_BEQ:
                    take = x[rs1] == x[rs2];
                    break;
                case rv32i::FUNCT3_BNE:
                    take = x[rs1] != x[rs2];
                    break;
                case rv32i::FUNCT3_BLT:
                    take = static_cast<int32_t>(x[rs1]) < static_cast<int32_t>(x[rs2]);
                    break;
                case rv32i::FUNCT3_BGE:
                    take = static_cast<int32_t>(x[rs1]) >= static_cast<int32_t>(x[rs2]);
                    break;
                case rv32i::FUNCT3_BLTU:
                    take = x[rs1] < x[rs2];
                    break;
                case rv32i::FUNCT3_BGEU:
                    take = x[rs1] >= x[rs2];
                    break;
                default:
                    std::cout << "[CPU] Branch nao implementado (funct3).\n";
                    break;
            }

            if (take) {
                next_pc = pc + imm;
                std::cout << "[CPU] Branch tomado para " << next_pc << std::endl;
            } else {
                std::cout << "[CPU] Branch NAO tomado" << std::endl;
            }
            break;
        }
        case rv32i::OPCODE_OP_IMM: {
            uint32_t imm12 = getBits(inst, 31, 20);
            int32_t imm = signExtend(imm12, 12);

            std::cout << "[CPU] (I-type) imm=" << imm << std::endl;

            switch (funct3) {
                case rv32i::FUNCT3_ADDI: {
                    uint32_t old = x[rs1];
                    uint32_t result = old + imm;

                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando ADDI: x" << rd
                              << " = x" << rs1 << "(" << old << ") + "
                              << imm << " -> " << result << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SLTI: {
                    int32_t lhs = static_cast<int32_t>(x[rs1]);
                    uint32_t result = lhs < imm ? 1u : 0u;
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLTI: x" << rd << " = " << lhs
                              << " < " << imm << " ? " << result << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SLTIU: {
                    uint32_t result = x[rs1] < static_cast<uint32_t>(imm) ? 1u : 0u;
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLTIU: x" << rd << " = x" << rs1
                              << " <u imm -> " << result << std::endl;
                    break;
                }
                case rv32i::FUNCT3_XORI: {
                    uint32_t result = x[rs1] ^ static_cast<uint32_t>(imm);
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando XORI: x" << rd << " = x" << rs1
                              << " XOR " << imm << std::endl;
                    break;
                }
                case rv32i::FUNCT3_ORI: {
                    uint32_t result = x[rs1] | static_cast<uint32_t>(imm);
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando ORI: x" << rd << " = x" << rs1
                              << " OR " << imm << std::endl;
                    break;
                }
                case rv32i::FUNCT3_ANDI: {
                    uint32_t result = x[rs1] & static_cast<uint32_t>(imm);
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando ANDI: x" << rd << " = x" << rs1
                              << " AND " << imm << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SLLI: {
                    uint32_t shamt = getBits(inst, 24, 20);
                    uint32_t result = x[rs1] << shamt;
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLLI: x" << rd << " = x" << rs1
                              << " << " << shamt << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SRLI_SRAI: {
                    uint32_t shamt = getBits(inst, 24, 20);
                    if (funct7 == rv32i::FUNCT7_SRA) {
                        int32_t result = static_cast<int32_t>(x[rs1]) >> shamt;
                        if (rd != 0) {
                            x[rd] = static_cast<uint32_t>(result);
                        }
                        std::cout << "[CPU] Executando SRAI: x" << rd << " = x" << rs1
                                  << " >>a " << shamt << std::endl;
                    } else {
                        uint32_t result = x[rs1] >> shamt;
                        if (rd != 0) {
                            x[rd] = result;
                        }
                        std::cout << "[CPU] Executando SRLI: x" << rd << " = x" << rs1
                                  << " >> " << shamt << std::endl;
                    }
                    break;
                }
                default:
                    std::cout << "[CPU] I-type nao implementado.\n";
                    break;
            }
            break;
        }
        case rv32i::OPCODE_OP: {
            switch (funct3) {
                case rv32i::FUNCT3_ADD_SUB:
                    if (funct7 == rv32i::FUNCT7_ADD) {
                        uint32_t a = x[rs1];
                        uint32_t b = x[rs2];
                        uint32_t result = a + b;
                        if (rd != 0) {
                            x[rd] = result;
                        }
                        std::cout << "[CPU] Executando ADD: x" << rd
                                  << " = x" << rs1 << "(" << a << ") + "
                                  << "x" << rs2 << "(" << b << ") -> "
                                  << result << std::endl;
                    } else if (funct7 == rv32i::FUNCT7_SUB) {
                        uint32_t a = x[rs1];
                        uint32_t b = x[rs2];
                        uint32_t result = a - b;
                        if (rd != 0) {
                            x[rd] = result;
                        }
                        std::cout << "[CPU] Executando SUB: x" << rd
                                  << " = x" << rs1 << "(" << a << ") - "
                                  << "x" << rs2 << "(" << b << ") -> "
                                  << result << std::endl;
                    } else {
                        std::cout << "[CPU] R-type nao implementado (funct7).\n";
                    }
                    break;
                case rv32i::FUNCT3_SLL: {
                    uint32_t shamt = getBits(inst, 24, 20);
                    uint32_t result = x[rs1] << shamt;
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLL: x" << rd << " = x" << rs1
                              << " << " << shamt << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SLT: {
                    uint32_t result = static_cast<int32_t>(x[rs1]) <
                                      static_cast<int32_t>(x[rs2]);
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLT" << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SLTU: {
                    uint32_t result = x[rs1] < x[rs2];
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando SLTU" << std::endl;
                    break;
                }
                case rv32i::FUNCT3_XOR: {
                    uint32_t result = x[rs1] ^ x[rs2];
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando XOR" << std::endl;
                    break;
                }
                case rv32i::FUNCT3_SRL_SRA:
                    if (funct7 == rv32i::FUNCT7_SRA) {
                        int32_t result = static_cast<int32_t>(x[rs1]) >>
                                         static_cast<int32_t>(x[rs2] & 0x1F);
                        if (rd != 0) {
                            x[rd] = static_cast<uint32_t>(result);
                        }
                        std::cout << "[CPU] Executando SRA" << std::endl;
                    } else {
                        uint32_t result = x[rs1] >> (x[rs2] & 0x1F);
                        if (rd != 0) {
                            x[rd] = result;
                        }
                        std::cout << "[CPU] Executando SRL" << std::endl;
                    }
                    break;
                case rv32i::FUNCT3_OR: {
                    uint32_t result = x[rs1] | x[rs2];
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando OR" << std::endl;
                    break;
                }
                case rv32i::FUNCT3_AND: {
                    uint32_t result = x[rs1] & x[rs2];
                    if (rd != 0) {
                        x[rd] = result;
                    }
                    std::cout << "[CPU] Executando AND" << std::endl;
                    break;
                }
                default:
                    std::cout << "[CPU] R-type nao implementado (funct3).\n";
                    break;
            }
            break;
        }
        case rv32i::OPCODE_LOAD: {
            uint32_t imm12 = getBits(inst, 31, 20);
            int32_t imm = signExtend(imm12, 12);
            uint32_t addr = x[rs1] + imm;

            switch (funct3) {
                case rv32i::FUNCT3_LW:
                    if (addr % 4 != 0) {
                        std::cout << "[CPU] EXC: LW desalinhado em " << addr << " (ignorado)\n";
                    } else {
                        uint32_t value = bus.read32(addr);
                        if (rd != 0) {
                            x[rd] = value;
                        }
                        std::cout << "[CPU] Executando LW: x" << rd
                                  << " = MEM[" << addr << "] -> "
                                  << value << std::endl;
                    }
                    break;
                case rv32i::FUNCT3_LB: {
                    uint8_t raw = bus.read8(addr);
                    int32_t value = signExtend(raw, 8);
                    if (rd != 0) {
                        x[rd] = static_cast<uint32_t>(value);
                    }
                    std::cout << "[CPU] Executando LB: x" << rd << " = MEM[" << addr
                              << "] -> " << value << std::endl;
                    break;
                }
                case rv32i::FUNCT3_LBU: {
                    uint8_t raw = bus.read8(addr);
                    if (rd != 0) {
                        x[rd] = raw;
                    }
                    std::cout << "[CPU] Executando LBU: x" << rd << " = MEM[" << addr
                              << "] -> " << static_cast<uint32_t>(raw) << std::endl;
                    break;
                }
                case rv32i::FUNCT3_LH: {
                    if (addr % 2 != 0) {
                        std::cout << "[CPU] EXC: LH desalinhado em " << addr << " (ignorado)\n";
                    } else {
                        uint16_t raw = bus.read8(addr) | (bus.read8(addr + 1) << 8);
                        int32_t value = signExtend(raw, 16);
                        if (rd != 0) {
                            x[rd] = static_cast<uint32_t>(value);
                        }
                        std::cout << "[CPU] Executando LH: x" << rd << " = MEM[" << addr
                                  << "] -> " << value << std::endl;
                    }
                    break;
                }
                case rv32i::FUNCT3_LHU: {
                    if (addr % 2 != 0) {
                        std::cout << "[CPU] EXC: LHU desalinhado em " << addr << " (ignorado)\n";
                    } else {
                        uint16_t raw = bus.read8(addr) | (bus.read8(addr + 1) << 8);
                        if (rd != 0) {
                            x[rd] = raw;
                        }
                        std::cout << "[CPU] Executando LHU: x" << rd << " = MEM[" << addr
                                  << "] -> " << raw << std::endl;
                    }
                    break;
                }
                default:
                    std::cout << "[CPU] Load nao implementado.\n";
                    break;
            }
            break;
        }
        case rv32i::OPCODE_STORE: {
            uint32_t imm_high = getBits(inst, 31, 25);
            uint32_t imm_low = getBits(inst, 11, 7);
            uint32_t imm12u = (imm_high << 5) | imm_low;
            int32_t imm = signExtend(imm12u, 12);
            uint32_t addr = x[rs1] + imm;

            switch (funct3) {
                case rv32i::FUNCT3_SW:
                    if (addr % 4 != 0) {
                        std::cout << "[CPU] EXC: SW desalinhado em " << addr << " (ignorado)\n";
                    } else {
                        uint32_t value = x[rs2];
                        bus.write32(addr, value);
                        std::cout << "[CPU] Executando SW: MEM[" << addr << "] = x"
                                  << rs2 << "(" << value << ")\n";
                    }
                    break;
                case rv32i::FUNCT3_SB: {
                    uint8_t value = static_cast<uint8_t>(x[rs2] & 0xFF);
                    bus.write8(addr, value);
                    std::cout << "[CPU] Executando SB: MEM[" << addr << "] = x"
                              << rs2 << "(" << static_cast<uint32_t>(value) << ")\n";
                    break;
                }
                case rv32i::FUNCT3_SH: {
                    if (addr % 2 != 0) {
                        std::cout << "[CPU] EXC: SH desalinhado em " << addr << " (ignorado)\n";
                    } else {
                        uint16_t value = static_cast<uint16_t>(x[rs2] & 0xFFFF);
                        bus.write8(addr, static_cast<uint8_t>(value & 0xFF));
                        bus.write8(addr + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
                        std::cout << "[CPU] Executando SH: MEM[" << addr << "] = x"
                                  << rs2 << "(" << value << ")\n";
                    }
                    break;
                }
                default:
                    std::cout << "[CPU] Store nao implementado.\n";
                    break;
            }
            break;
        }
        default: {
            // Instrucao especial para retorno de interrupcao (MRET: 0x30200073)
            if (inst == 0x30200073) {
                if (in_interrupt) {
                    std::cout << "[CPU] MRET: retornando da interrupcao para PC="
                              << saved_pc << "\n";
                    next_pc = saved_pc;
                    in_interrupt = false;
                } else {
                    std::cout << "[CPU] MRET executado sem interrupcao ativa (ignorado)\n";
                }
            } else {
                std::cout << "[CPU] Instrucao desconhecida. NOP.\n";
            }
            break;
        }
    }
    pc = next_pc;
    x[0] = 0;

    // Atualiza contador de instrucoes
    instr_count++;

    // --- checagem de interrupcao via barramento ---
    if (!in_interrupt && bus.has_pending_interrupt()) {
        std::cout << "[CPU] Interrupcao detectada. Salvando PC=" << pc
                  << " e desviando para vetor 0x"
                  << std::hex << IRQ_VECTOR << std::dec << "\n";

        saved_pc    = pc;
        pc          = IRQ_VECTOR;
        in_interrupt = true;
        bus.clear_interrupt();
    }

    if (instr_count % VRAM_DUMP_INTERVAL == 0) {
        std::cout << "\n[CPU] Dump de VRAM apos " << instr_count
                  << " instrucoes executadas:\n";
        bus.dump_vram_ascii();
        std::cout << "\n";
    }
}
