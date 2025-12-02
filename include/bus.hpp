#pragma once

#include "memory.hpp"
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <stdexcept>

// Interface simples para um dispositivo de E/S (periférico)
class IoDevice {
public:
    virtual ~IoDevice() = default;

    virtual uint8_t read8(uint32_t address) = 0;
    virtual void write8(uint32_t address, uint8_t value) = 0;

    // Interrupção: por padrão, nenhum dispositivo gera IRQ
    virtual bool has_interrupt() const { return false; }
    virtual void clear_interrupt()     {}
};

// Barramento: decide para onde vai cada acesso de memória
// (RAM, VRAM, área reservada ou periférico).
class Bus {
private:
    Memory& ram;
    Memory& vram;
    IoDevice& io;

    // Mapa de memória conforme especificação
    static constexpr uint32_t RAM_START  = 0x00000;
    static constexpr uint32_t RAM_END    = 0x7FFFF; // inclusive

    static constexpr uint32_t VRAM_START = 0x80000;
    static constexpr uint32_t VRAM_END   = 0x8FFFF; // inclusive

    static constexpr uint32_t RSV_START  = 0x90000;
    static constexpr uint32_t RSV_END    = 0x9FBFF; // inclusive

    static constexpr uint32_t IO_START   = 0x9FC00;
    static constexpr uint32_t IO_END     = 0x9FFFF; // inclusive

    static constexpr uint32_t MEM_LIMIT  = 0xA0000; // limite superior (exclusivo)

    static bool is_in_range(uint32_t addr, uint32_t start, uint32_t end);

public:
    Bus(Memory& ram_mem, Memory& vram_mem, IoDevice& io_dev);

    uint8_t  read8(uint32_t address);
    void     write8(uint32_t address, uint8_t value);

    uint32_t read32(uint32_t address);
    void     write32(uint32_t address, uint32_t value);

    // Dump ASCII da VRAM (ja tinhamos colocado)
    void dump_vram_ascii(std::size_t cols = 32) const;

    // --- NOVO: interface de interrupção para a CPU ---
    bool has_pending_interrupt() const;
    void clear_interrupt();
};
