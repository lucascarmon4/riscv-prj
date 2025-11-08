#include "memory.hpp"

Memory::Memory(std::size_t size) 
        : data(size, 0) {}

uint8_t Memory::read(std::size_t address) const {
    return data.at(address);
}

void Memory::write(std::size_t address, uint8_t value) {
    data.at(address) = value;
}

void Memory::dump(std::size_t start, std::size_t count) const{
    std::cout << "Dump de Memoria [" << start << " .. " << (start + count - 1) << "]: ";
    for (std::size_t i = 0; i < count; ++i) {
        std::cout << (int)data.at(start + i) << " ";
    }
    std::cout << std::endl;
}