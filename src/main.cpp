#include <iostream>
#include "memory.hpp"
#include "bus.hpp"
#include "cpu.hpp"

uint32_t encodeADDI(uint8_t rd, uint8_t rs1, int32_t imm){
    uint32_t opcode = 0x13;
    uint32_t funct3 = 0x0;
    uint32_t imm12 = static_cast<uint32_t>(imm & 0xFFF);

    return (imm12 << 20)
        | (static_cast<uint32_t>(rs1) << 15)
        | (funct3 << 12)
        | (static_cast<uint32_t>(rd) << 7)
        | opcode;
}

uint32_t encodeR(uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7){
    uint32_t opcode = 0x33;
    return (static_cast<uint32_t>(funct7) << 25)
        | (static_cast<uint32_t>(rs2) << 20)
        | (static_cast<uint32_t>(rs1) << 15)
        | (static_cast<uint32_t>(funct3) << 12)
        | (static_cast<uint32_t>(rd) << 7)
        | opcode;
}

int main() {
    std::cout << "Iniciando teste (CPU ADDI/ADD/SUB)\n" << std::endl;

    Memory mem(128);
    Bus bus(mem);   
    CPU cpu(bus);

    uint32_t inst0 = encodeADDI(1, 0, 5);
    uint32_t inst1 = encodeADDI(2, 0, 10);
    uint32_t inst2 = encodeR(3, 1, 2, 0x0, 0x00);
    uint32_t inst3 = encodeR(4, 3, 1, 0x0, 0x20);

    mem.write32(0x00, inst0);
    mem.write32(0x04, inst1);
    mem.write32(0x08, inst2);
    mem.write32(0x0C, inst3);

    std::cout << "\nDump inicial da CPU:\n";
    cpu.dump();

    std::cout << "\nExecutando 4 instrucoes:\n";
    cpu.step();
    cpu.step();
    cpu.step();
    cpu.step();

    std::cout << "\nDump final da CPU:\n";
    cpu.dump();

    return 0;
}