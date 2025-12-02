#pragma once

#include "bus.hpp"
#include <cstdint>
#include <cstddef>
#include <iostream>

// Periferico simples "fake" apenas para cumprir a regiao de IO
class FakeIO : public IoDevice {
private:
    static constexpr std::size_t IO_SIZE  = 1024; // 1 KiB
    static constexpr uint32_t    IRQ_REG  = 0;    // primeiro byte = flag de interrupcao

    uint8_t regs[IO_SIZE]{}; // zera tudo

public:
    uint8_t read8(uint32_t address) override {
        if (address >= IO_SIZE) {
            std::cout << "[IO] read8 fora do limite: " << address << std::endl;
            return 0;
        }
        std::cout << "[IO] read8 offset=" << address << std::endl;
        return regs[address];
    }

    void write8(uint32_t address, uint8_t value) override {
        if (address >= IO_SIZE) {
            std::cout << "[IO] write8 fora do limite: " << address
                      << " value=" << static_cast<int>(value) << std::endl;
            return;
        }
        std::cout << "[IO] write8 offset=" << address
                  << " value=" << static_cast<int>(value) << std::endl;
        regs[address] = value;
    }

    // Quando regs[IRQ_REG] != 0, consideramos que ha interrupcao pendente
    bool has_interrupt() const override {
        return regs[IRQ_REG] != 0;
    }

    void clear_interrupt() override {
        regs[IRQ_REG] = 0;
    }
};
