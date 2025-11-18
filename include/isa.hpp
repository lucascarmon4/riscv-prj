#pragma once

#include <cstdint>

namespace rv32i {
// Opcodes
constexpr uint32_t OPCODE_OP_IMM = 0x13;
constexpr uint32_t OPCODE_OP = 0x33;
constexpr uint32_t OPCODE_LOAD = 0x03;
constexpr uint32_t OPCODE_STORE = 0x23;
constexpr uint32_t OPCODE_LUI = 0x37;
constexpr uint32_t OPCODE_AUIPC = 0x17;
constexpr uint32_t OPCODE_JAL = 0x6F;
constexpr uint32_t OPCODE_JALR = 0x67;
constexpr uint32_t OPCODE_BRANCH = 0x63;

// Funct3
constexpr uint32_t FUNCT3_ADDI = 0x0;
constexpr uint32_t FUNCT3_SLTI = 0x2;
constexpr uint32_t FUNCT3_SLTIU = 0x3;
constexpr uint32_t FUNCT3_XORI = 0x4;
constexpr uint32_t FUNCT3_ORI = 0x6;
constexpr uint32_t FUNCT3_ANDI = 0x7;
constexpr uint32_t FUNCT3_SLLI = 0x1;
constexpr uint32_t FUNCT3_SRLI_SRAI = 0x5;

constexpr uint32_t FUNCT3_LW = 0x2;
constexpr uint32_t FUNCT3_LB = 0x0;
constexpr uint32_t FUNCT3_LH = 0x1;
constexpr uint32_t FUNCT3_LBU = 0x4;
constexpr uint32_t FUNCT3_LHU = 0x5;

constexpr uint32_t FUNCT3_SW = 0x2;
constexpr uint32_t FUNCT3_SB = 0x0;
constexpr uint32_t FUNCT3_SH = 0x1;

constexpr uint32_t FUNCT3_ADD_SUB = 0x0;
constexpr uint32_t FUNCT3_SLL = 0x1;
constexpr uint32_t FUNCT3_SLT = 0x2;
constexpr uint32_t FUNCT3_SLTU = 0x3;
constexpr uint32_t FUNCT3_XOR = 0x4;
constexpr uint32_t FUNCT3_SRL_SRA = 0x5;
constexpr uint32_t FUNCT3_OR = 0x6;
constexpr uint32_t FUNCT3_AND = 0x7;

constexpr uint32_t FUNCT3_BEQ = 0x0;
constexpr uint32_t FUNCT3_BNE = 0x1;
constexpr uint32_t FUNCT3_BLT = 0x4;
constexpr uint32_t FUNCT3_BGE = 0x5;
constexpr uint32_t FUNCT3_BLTU = 0x6;
constexpr uint32_t FUNCT3_BGEU = 0x7;

// Funct7
constexpr uint32_t FUNCT7_ADD = 0x00;
constexpr uint32_t FUNCT7_SUB = 0x20;
constexpr uint32_t FUNCT7_SRA = 0x20;
constexpr uint32_t FUNCT7_SRL_SLL = 0x00;
}  // namespace rv32i
