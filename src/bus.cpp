#include "bus.hpp"

bool Bus::is_in_range(uint32_t addr, uint32_t start, uint32_t end) {
    return addr >= start && addr <= end;
}

Bus::Bus(Memory& ram_mem, Memory& vram_mem, IoDevice& io_dev)
    : ram(ram_mem),
      vram(vram_mem),
      io(io_dev) {}

// Leitura de 1 byte
uint8_t Bus::read8(uint32_t address) {
    std::cout << "Bus: lendo byte no endereco " << address << std::endl;

    if (address >= MEM_LIMIT) {
        throw std::out_of_range("Endereco de memoria fora do limite fisico");
    }

    if (is_in_range(address, RAM_START, RAM_END)) {
        // RAM: endereco fisico == indice no vetor
        uint32_t offset = address - RAM_START;
        return ram.read8(offset);
    }

    if (is_in_range(address, VRAM_START, VRAM_END)) {
        // VRAM: ajusta endereco para o inicio da VRAM (0)
        uint32_t offset = address - VRAM_START;
        return vram.read8(offset);
    }

    if (is_in_range(address, RSV_START, RSV_END)) {
        // Area reservada: acesso proibido
        throw std::runtime_error("Leitura em area de memoria reservada");
    }

    if (is_in_range(address, IO_START, IO_END)) {
        // Periferico mapeado em memoria
        uint32_t offset = address - IO_START;
        return io.read8(offset);
    }

    // Se chegou aqui, algo deu errado no mapa
    throw std::runtime_error("Endereco de memoria nao mapeado");
}

// Escrita de 1 byte
void Bus::write8(uint32_t address, uint8_t value) {
    std::cout << "Bus: escrevendo byte " << static_cast<int>(value)
              << " no endereco " << address << std::endl;

    if (address >= MEM_LIMIT) {
        throw std::out_of_range("Endereco de memoria fora do limite fisico");
    }

    if (is_in_range(address, RAM_START, RAM_END)) {
        uint32_t offset = address - RAM_START;
        ram.write8(offset, value);
        return;
    }

    if (is_in_range(address, VRAM_START, VRAM_END)) {
        uint32_t offset = address - VRAM_START;
        vram.write8(offset, value);
        return;
    }

    if (is_in_range(address, RSV_START, RSV_END)) {
        throw std::runtime_error("Escrita em area de memoria reservada");
    }

    if (is_in_range(address, IO_START, IO_END)) {
        uint32_t offset = address - IO_START;
        io.write8(offset, value);
        return;
    }

    throw std::runtime_error("Endereco de memoria nao mapeado");
}

// Leitura de 32 bits (4 bytes)
uint32_t Bus::read32(uint32_t address) {
    std::cout << "Bus: lendo palavra 32 bits no endereco " << address << std::endl;

    // Usa quatro leituras de byte, respeitando o mapeamento
    uint32_t b0 = read8(address);
    uint32_t b1 = read8(address + 1);
    uint32_t b2 = read8(address + 2);
    uint32_t b3 = read8(address + 3);

    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

// Escrita de 32 bits (4 bytes)
void Bus::write32(uint32_t address, uint32_t value) {
    std::cout << "Bus: escrevendo palavra 32 bits = "
              << value << " no endereco " << address << std::endl;

    // Quebra o valor em 4 bytes e usa write8,
    // que ja sabe qual regiao de memoria usar.
    uint8_t b0 = static_cast<uint8_t>(value & 0xFF);
    uint8_t b1 = static_cast<uint8_t>((value >> 8) & 0xFF);
    uint8_t b2 = static_cast<uint8_t>((value >> 16) & 0xFF);
    uint8_t b3 = static_cast<uint8_t>((value >> 24) & 0xFF);

    write8(address,     b0);
    write8(address + 1, b1);
    write8(address + 2, b2);
    write8(address + 3, b3);
}

// Novo: dump ASCII da VRAM inteira
void Bus::dump_vram_ascii(std::size_t cols) const {
    std::cout << "[BUS] Dump ASCII da VRAM\n";

    // Tamanho da VRAM = VRAM_END - VRAM_START + 1 = 0x10000
    constexpr uint32_t VRAM_SIZE = VRAM_END - VRAM_START + 1;

    for (uint32_t i = 0; i < VRAM_SIZE; ++i) {
        uint8_t byte = vram.read8(i);

        char c;
        if (byte == 0) {
            c = ' '; // trata 0 como espaço em branco
        } else if (byte >= 32 && byte <= 126) {
            c = static_cast<char>(byte); // ASCII imprimivel
        } else {
            c = '.'; // qualquer coisa não-imprimível vira ponto
        }

        std::cout << c;

        if ((i + 1) % cols == 0) {
            std::cout << '\n';
        }
    }

    if (VRAM_SIZE % cols != 0) {
        std::cout << '\n';
    }

    std::cout << "[BUS] Fim do dump da VRAM\n";
}

bool Bus::has_pending_interrupt() const {
    return io.has_interrupt();
}

void Bus::clear_interrupt() {
    io.clear_interrupt();
}
