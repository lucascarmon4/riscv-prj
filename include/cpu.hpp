#pragma once
#include "bus.hpp"
#include <array>
#include <cstdint>
#include <iostream>

class CPU {
    private:
        std::array<uint32_t, 32> x{};
        uint32_t pc{0};
        Bus& bus;

        // Contador de instrucoes executadas
        uint64_t instr_count{0};
        static constexpr uint64_t VRAM_DUMP_INTERVAL = 10;

        // --- NOVO: suporte simples a interrupcao ---
        bool     in_interrupt{false};
        uint32_t saved_pc{0};
        static constexpr uint32_t IRQ_VECTOR = 0x00000100; // endereco do handler

        uint32_t getBits(uint32_t value, int hi, int lo) const;
        int32_t signExtend(uint32_t value, int bits) const;

    public:
        explicit CPU(Bus& b);

        void reset();
        void dump() const;
        void step();
};
