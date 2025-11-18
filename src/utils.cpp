#include "utils.hpp"

#include "isa.hpp"

uint32_t encodeADDI(uint8_t rd, uint8_t rs1, int32_t imm) {
    const uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFFu;

    return (imm12 << 20) | (static_cast<uint32_t>(rs1) << 15) |
           (rv32i::FUNCT3_ADDI << 12) | (static_cast<uint32_t>(rd) << 7) |
           rv32i::OPCODE_OP_IMM;
}

uint32_t encodeLW(uint8_t rd, uint8_t rs1, int32_t imm) {
    const uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFFu;

    return (imm12 << 20) | (static_cast<uint32_t>(rs1) << 15) |
           (rv32i::FUNCT3_LW << 12) | (static_cast<uint32_t>(rd) << 7) |
           rv32i::OPCODE_LOAD;
}

uint32_t encodeSW(uint8_t rs2, uint8_t rs1, int32_t imm) {
    const uint32_t imm12 = static_cast<uint32_t>(imm) & 0xFFFu;
    const uint32_t imm_low = imm12 & 0x1Fu;
    const uint32_t imm_high = (imm12 >> 5) & 0x7Fu;

    return (imm_high << 25) | (static_cast<uint32_t>(rs2) << 20) |
           (static_cast<uint32_t>(rs1) << 15) | (rv32i::FUNCT3_SW << 12) |
           (imm_low << 7) | rv32i::OPCODE_STORE;
}

uint32_t encodeR(uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3,
                 uint8_t funct7) {
    return (static_cast<uint32_t>(funct7) << 25) |
           (static_cast<uint32_t>(rs2) << 20) |
           (static_cast<uint32_t>(rs1) << 15) |
           (static_cast<uint32_t>(funct3) << 12) |
            (static_cast<uint32_t>(rd) << 7) | rv32i::OPCODE_OP;
}

uint32_t encodeB(uint8_t rs1, uint8_t rs2, uint8_t funct3, int32_t imm) {
    // imm is a signed byte offset; bit0 is always zero (halfword alignment)
    uint32_t imm12 = (static_cast<uint32_t>(imm) >> 12) & 0x1;
    uint32_t imm10_5 = (static_cast<uint32_t>(imm) >> 5) & 0x3F;
    uint32_t imm4_1 = (static_cast<uint32_t>(imm) >> 1) & 0xF;
    uint32_t imm11 = (static_cast<uint32_t>(imm) >> 11) & 0x1;

    return (imm12 << 31) | (imm11 << 7) | (imm10_5 << 25) | (imm4_1 << 8) |
           (static_cast<uint32_t>(rs2) << 20) |
           (static_cast<uint32_t>(rs1) << 15) |
           (static_cast<uint32_t>(funct3) << 12) |
           rv32i::OPCODE_BRANCH;
}

uint32_t encodeJAL(uint8_t rd, int32_t imm) {
    // imm is a signed byte offset, bit0 is zero
    uint32_t imm20 = (static_cast<uint32_t>(imm) >> 20) & 0x1;
    uint32_t imm10_1 = (static_cast<uint32_t>(imm) >> 1) & 0x3FF;
    uint32_t imm11 = (static_cast<uint32_t>(imm) >> 11) & 0x1;
    uint32_t imm19_12 = (static_cast<uint32_t>(imm) >> 12) & 0xFF;

    return (imm20 << 31) | (imm19_12 << 12) | (imm11 << 20) |
           (imm10_1 << 21) | (static_cast<uint32_t>(rd) << 7) |
           rv32i::OPCODE_JAL;
}

uint32_t encodeLUI(uint8_t rd, int32_t imm) {
    // imm already aligned to upper 20 bits
    return (static_cast<uint32_t>(imm) & 0xFFFFF000) |
           (static_cast<uint32_t>(rd) << 7) |
           rv32i::OPCODE_LUI;
}
