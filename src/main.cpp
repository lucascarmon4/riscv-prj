#include <iostream>

#include "bus.hpp"
#include "cpu.hpp"
#include "isa.hpp"
#include "memory.hpp"
#include "utils.hpp"

int main() {
    std::cout << "Iniciando teste com branches, jumps e stores\n";

    Memory mem(256);
    Bus bus(mem);
    CPU cpu(bus);

    uint32_t inst0 = encodeADDI(10, 0, 64);          // x10 = base para SW/LW
    uint32_t inst1 = encodeADDI(1, 0, 0);            // x1 = acumulador
    uint32_t inst2 = encodeADDI(2, 0, 5);            // x2 = contador
    uint32_t inst3 = encodeADDI(3, 0, 1);            // x3 = incremento
    uint32_t inst4 = encodeR(1, 1, 3, rv32i::FUNCT3_ADD_SUB,
                            rv32i::FUNCT7_ADD);     // x1 += x3
    uint32_t inst5 = encodeADDI(2, 2, -1);           // x2--
    uint32_t inst6 = encodeB(2, 0, rv32i::FUNCT3_BNE, -8); // enquanto x2 != 0
    uint32_t inst7 = encodeSW(1, 10, 0);             // MEM[x10] = x1
    uint32_t inst8 = encodeLW(4, 10, 0);             // x4 = MEM[x10]

    mem.write32(0x00, inst0);
    mem.write32(0x04, inst1);
    mem.write32(0x08, inst2);
    mem.write32(0x0C, inst3);
    mem.write32(0x10, inst4);
    mem.write32(0x14, inst5);
    mem.write32(0x18, inst6);
    mem.write32(0x1C, inst7);
    mem.write32(0x20, inst8);

    std::cout << "\nDump inicial da CPU:\n";
    cpu.dump();

    std::cout << "\nExecutando 25 instrucoes:\n";
    for (int i = 0; i < 25; ++i) {
        cpu.step();
    }

    std::cout << "\nDump final da CPU:\n";
    cpu.dump();

    return 0;
}