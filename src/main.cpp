#include <iostream>
#include <limits>
#include <conio.h> // _kbhit, _getch (Windows)

#include "bus.hpp"
#include "cpu.hpp"
#include "isa.hpp"
#include "memory.hpp"
#include "utils.hpp"
#include "io_fake.hpp"

// Helper para LUI local
static uint32_t encodeLUI_local(uint32_t rd, uint32_t imm20) {
    return (imm20 << 12) | (rd << 7) | rv32i::OPCODE_LUI;
}

int main() {
    std::cout << "Simulador RV32I - Programa de teste + interrupcao via teclado + interatividade\n";

    Memory ram(0x80000);   // 0x00000..0x7FFFF
    Memory vram(0x10000);  // 0x80000..0x8FFFF
    FakeIO io;
    Bus bus(ram, vram, io);
    CPU cpu(bus);

    // ------------------------------------------------------------------
    // PROGRAMA PRINCIPAL EM RISC-V (base = 0x00000000)
    //
    // Parte 1: algoritmo simples em RAM
    //   x10 = 64        // base RAM
    //   x1  = 0         // acumulador
    //   x2  = 5         // contador
    //   x3  = 1         // incremento
    // loop:
    //   x1 = x1 + x3
    //   RAM[x10] = x1
    //   x4 = RAM[x10]
    //   x2--
    //   if (x2 != 0) goto loop
    // ------------------------------------------------------------------

    uint32_t addr = 0x00000000;

    uint32_t inst0 = encodeADDI(10, 0, 64);          // x10 = 64 (base RAM)
    uint32_t inst1 = encodeADDI(1,  0, 0);           // x1 = 0
    uint32_t inst2 = encodeADDI(2,  0, 5);           // x2 = 5
    uint32_t inst3 = encodeADDI(3,  0, 1);           // x3 = 1
    uint32_t inst4 = encodeR(1, 1, 3,
                             rv32i::FUNCT3_ADD_SUB,
                             rv32i::FUNCT7_ADD);     // x1 = x1 + x3
    uint32_t inst5 = encodeADDI(2, 2, -1);           // x2--
    uint32_t inst6 = encodeB(2, 0, rv32i::FUNCT3_BNE,
                             -8);                    // enquanto x2 != 0, volta
    uint32_t inst7 = encodeSW(1, 10, 0);             // RAM[x10] = x1
    uint32_t inst8 = encodeLW(4, 10, 0);             // x4 = RAM[x10]

    ram.write32(addr + 0x00, inst0);
    ram.write32(addr + 0x04, inst1);
    ram.write32(addr + 0x08, inst2);
    ram.write32(addr + 0x0C, inst3);
    ram.write32(addr + 0x10, inst4);
    ram.write32(addr + 0x14, inst5);
    ram.write32(addr + 0x18, inst6);
    ram.write32(addr + 0x1C, inst7);
    ram.write32(addr + 0x20, inst8);

    // ------------------------------------------------------------------
    // Parte 2: E/S programada com VRAM
    //
    //   x20 = 0x80000        // base VRAM
    //   x21 = 'A'
    //   x22 = 'B'
    //   x23 = 'C'
    //   VRAM[0]  = 'A'
    //   VRAM[4]  = 'B'
    //   VRAM[8]  = 'C'
    // ------------------------------------------------------------------

    const uint32_t VRAM_BASE_IMM20 = 0x00080; // 0x80000 >> 12

    uint32_t inst9  = encodeLUI_local(20, VRAM_BASE_IMM20); // x20 = 0x80000
    uint32_t inst10 = encodeADDI(21, 0, 65);                // 'A'
    uint32_t inst11 = encodeADDI(22, 0, 66);                // 'B'
    uint32_t inst12 = encodeADDI(23, 0, 67);                // 'C'
    uint32_t inst13 = encodeSW(21, 20, 0);                  // VRAM[0] = 'A'
    uint32_t inst14 = encodeSW(22, 20, 4);                  // VRAM[4] = 'B'
    uint32_t inst15 = encodeSW(23, 20, 8);                  // VRAM[8] = 'C'

    ram.write32(addr + 0x24, inst9);
    ram.write32(addr + 0x28, inst10);
    ram.write32(addr + 0x2C, inst11);
    ram.write32(addr + 0x30, inst12);
    ram.write32(addr + 0x34, inst13);
    ram.write32(addr + 0x38, inst14);
    ram.write32(addr + 0x3C, inst15);

    // ------------------------------------------------------------------
    // Parte 3: IO mapeado + interrupcao externa
    //
    //   x30 = 0x9FC00        // base IO
    //   x5  = 1
    //   IO[0] = 1            // seta flag de interrupcao no FakeIO
    //
    // Essa escrita pode ser feita pelo proprio programa
    // OU externamente pela tecla 'k' (via host).
    // ------------------------------------------------------------------

    const uint32_t IO_BASE_IMM20 = 0x0009F; // 0x9FC00 >> 12 = 0x9F, resto 0xC00

    uint32_t inst16 = encodeLUI_local(30, IO_BASE_IMM20);   // x30 = 0x9F000
    uint32_t inst17 = encodeADDI(30, 30, 0xC00);            // x30 = 0x9FC00
    uint32_t inst18 = encodeADDI(5,  0, 1);                 // x5 = 1
    uint32_t inst19 = encodeSW(5, 30, 0);                   // IO[0] = 1 (IRQ)

    // Loop infinito no programa principal depois de disparar interrupcao
    uint32_t inst20 = encodeB(0, 0, rv32i::FUNCT3_BEQ, 0);  // BEQ x0,x0,0

    ram.write32(addr + 0x40, inst16);
    ram.write32(addr + 0x44, inst17);
    ram.write32(addr + 0x48, inst18);
    ram.write32(addr + 0x4C, inst19);
    ram.write32(addr + 0x50, inst20);

    // ------------------------------------------------------------------
    // Handler de interrupcao no vetor 0x00000100 (IRQ_VECTOR da CPU)
    //
    //   x6 = 'H'
    //   x7 = '!' (33)
    //   VRAM[12] = 'H'
    //   VRAM[16] = '!'
    //   MRET (0x30200073) -> retorna para o PC salvo
    // ------------------------------------------------------------------

    const uint32_t IRQ_VECTOR = 0x00000100;
    uint32_t haddr = IRQ_VECTOR;

    uint32_t h0 = encodeADDI(6, 0, 72);        // 'H'
    uint32_t h1 = encodeADDI(7, 0, 33);        // '!'
    uint32_t h2 = encodeSW(6, 20, 12);         // VRAM[12] = 'H'
    uint32_t h3 = encodeSW(7, 20, 16);         // VRAM[16] = '!'
    uint32_t h4 = 0x30200073;                  // MRET (retorno de interrupcao)

    ram.write32(haddr + 0x00, h0);
    ram.write32(haddr + 0x04, h1);
    ram.write32(haddr + 0x08, h2);
    ram.write32(haddr + 0x0C, h3);
    ram.write32(haddr + 0x10, h4);

    std::cout << "\n[MAIN] Estado inicial da CPU:\n";
    cpu.dump();

    // ---------------------- MENU INTERATIVO ----------------------
    while (true) {
        std::cout << "\n================ MENU ================\n";
        std::cout << "1) Executar 1 instrucao\n";
        std::cout << "2) Executar N instrucoes (tecla 'k' dispara IRQ externa)\n";
        std::cout << "3) Mostrar registradores (dump CPU)\n";
        std::cout << "4) Dump de RAM (primeiros 64 bytes)\n";
        std::cout << "5) Dump de VRAM ASCII (matriz 32 colunas)\n";
        std::cout << "6) Editar um byte da RAM\n";
        std::cout << "7) Editar um byte da VRAM\n";
        std::cout << "0) Sair\n";
        std::cout << "Escolha uma opcao: ";

        int opcao;
        if (!(std::cin >> opcao)) {
            std::cout << "Entrada invalida. Encerrando.\n";
            break;
        }

        if (opcao == 0) {
            std::cout << "Encerrando simulador.\n";
            break;
        }

        switch (opcao) {
            case 1: {
                cpu.step();
                break;
            }
            case 2: {
                std::cout << "Quantas instrucoes deseja executar? ";
                int n;
                if (!(std::cin >> n) || n <= 0) {
                    std::cout << "Valor invalido.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }

                for (int i = 0; i < n; ++i) {
                    cpu.step();

                    // Verifica se o usuario apertou alguma tecla
                    if (_kbhit()) {
                        int ch = _getch();
                        if (ch == 'k' || ch == 'K') {
                            std::cout << "[MAIN] Tecla 'k' pressionada: "
                                         "disparando interrupcao externa via IO e voltando ao menu\n";
                            io.write8(0, 1);   // seta IRQ no FakeIO
                            break;             // sai do loop ANTES de completar as n instrucoes
                        }
                    }
                }
                break;
            }
            case 3: {
                cpu.dump();
                break;
            }
            case 4: {
                std::cout << "[RAM] Dump dos primeiros 64 bytes:\n";
                ram.dump(0, 64);
                break;
            }
            case 5: {
                std::cout << "[VRAM] Dump ASCII (32 colunas):\n";
                bus.dump_vram_ascii(32);
                break;
            }
            case 6: {
                std::cout << "Endereco RAM (decimal, 0 a " << (0x80000 - 1) << "): ";
                uint32_t addr_ram;
                if (!(std::cin >> addr_ram) || addr_ram >= 0x80000) {
                    std::cout << "Endereco invalido.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cout << "Valor (0 a 255): ";
                int valor;
                if (!(std::cin >> valor) || valor < 0 || valor > 255) {
                    std::cout << "Valor invalido.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                ram.write8(addr_ram, static_cast<uint8_t>(valor));
                std::cout << "[RAM] Escrito " << valor << " em RAM[" << addr_ram << "]\n";
                break;
            }
            case 7: {
                std::cout << "Endereco VRAM absoluto (ex: 524288 para 0x80000): ";
                uint32_t addr_v;
                if (!(std::cin >> addr_v) || addr_v < 0x80000 || addr_v >= 0x90000) {
                    std::cout << "Endereco invalido (VRAM = 0x80000 .. 0x8FFFF).\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cout << "Valor (0 a 255, ASCII): ";
                int valor;
                if (!(std::cin >> valor) || valor < 0 || valor > 255) {
                    std::cout << "Valor invalido.\n";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                uint32_t offset = addr_v - 0x80000;
                vram.write8(offset, static_cast<uint8_t>(valor));
                std::cout << "[VRAM] Escrito " << valor << " em VRAM[" << addr_v
                          << "] (offset " << offset << ")\n";
                break;
            }
            default:
                std::cout << "Opcao invalida.\n";
                break;
        }
    }

    return 0;
}
