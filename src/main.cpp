#include <iostream>
#include <vector>
#include <cstdint>

struct Memory {
    std::vector<uint8_t> data;

    Memory(std::size_t size) 
            :  data(size, 0) {}

    uint8_t read(std::size_t address) const {
        return data.at(address);
    }

    void write(std::size_t address, uint8_t value) {
        data.at(address) = value;
    }

    void dump(std::size_t start, std::size_t count) const{
        std::cout << "Dump de Memoria [" << start << " .. " << (start + count - 1) << "]: ";
        for (std::size_t i = 0; i < count; ++i) {
            std::cout << (int)data.at(start + i) << " ";
        }
        std::cout << std::endl;
    }
};  

int main() {
    std::cout << "Iniciando teste de memória.../n";

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