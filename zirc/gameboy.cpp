#include <cstdint>
#include <fstream>
#include <stdexcept>

#include "cpu/cpu.hpp"
#include "gameboy.hpp"

namespace Zirc {
void GameBoy::start(void) {
	if(m_cartridge->type() != 0) { throw std::runtime_error("\"ROM only\" cartridges are supported.\n"); }

	for(;;) {
		m_cpu.executeInstruction();
	}
}

int GameBoy::tick() {
	m_cpu.handleInterrupts();
	uint8_t cycles = m_cpu.executeInstruction();
	assert(cycles % 4 == 0);

	for(uint8_t i = 0; i < cycles; i += 4) {
		m_ppu.tick(4);
		m_timer.tick(4);
		m_apu.tick(4);

		uint8_t div = m_timer.getDiv();
		if((m_prevDiv & 0x10) == 0x10 && (div & 0x10) == 0) { m_apu.increaseDiv(); }
		m_prevDiv = div;
	}

	return cycles;
}

void GameBoy::reset() {
	m_apu.reset();
	m_bus.reset();
	m_cpu.reset();
	m_joypad.reset();
	m_lcd.resetScreenX();
	m_memory.reset();
	m_ppu.reset();
	m_timer.reset();
	m_cartridge->reset();

	m_prevDiv = 0;
}

void GameBoy::loadCustomBootRom(const std::string &bootRomPath) {
	std::ifstream bootRomFile(bootRomPath, std::ios::binary | std::ios::ate);
	if(!bootRomFile) { throw std::runtime_error("Could not open the custom boot ROM file."); }

	std::streamsize bootRomSize = bootRomFile.tellg();
	if(bootRomSize != 256) { throw std::runtime_error("The boot ROM's size must be exactly 256 bytes long."); }

	std::array<char, 256> bootRom;
	bootRomFile.seekg(0, std::ios::beg);
	if(!bootRomFile.read(bootRom.data(), 256)) {
		throw std::runtime_error("Failed to read from the custom boot ROM file.");
	}

	m_cartridge->setCustomBootRom(reinterpret_cast<uint8_t *>(bootRom.data()));
}
} // namespace Zirc
