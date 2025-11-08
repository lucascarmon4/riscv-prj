#include <iostream>
#include "memory.hpp"

int main() {
    std::cout << "Iniciando teste de Memoria (modulo memoria)" << std::endl;

    Memory mem(64);
    mem.write(10, 42);
    mem.write(11, 99);
    mem.write(12, 7);

    std::cout << "Valor na posicao 10: " << (int)mem.read(10) << std::endl;
    std::cout << "Valor na posicao 11: " << (int)mem.read(11) << std::endl;
    std::cout << "Valor na posicao 12: " << (int)mem.read(12) << std::endl;

    mem.dump(8, 8);
    return 0;
}