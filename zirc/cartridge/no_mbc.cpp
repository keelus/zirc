#include <iomanip>
#include <iostream>
#include <sstream>

#include "../common.hpp"
#include "no_mbc.hpp"

namespace Zirc {

uint8_t NoMBC::read8(const uint16_t address) const {
	if(address <= 0x7FFF) {
		if(isBootRomMapped() && address < 0x100) {
			return read8BootRom(address);
		} else {
			return m_rom[address];
		}
	} else if(IN_RANGE(address, 0xA000, 0xBFFF)) {
		return m_ram[address - 0xA000];
	} else {
		std::stringstream stream;
		stream << "Cartridge[NoMBC]: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << int(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
}

void NoMBC::write8(const uint16_t address, const uint8_t value) {
	if(address <= 0x7FFF) {
		std::cout << "Cartridge[NoMBC]: Ignoring write to Cartridge's ROM at address 0x" << std::hex << std::setw(4)
				  << std::setfill('0') << address << " with value 0x" << std::setw(2) << value << std::endl;
		return;
	} else if(IN_RANGE(address, 0xA000, 0xBFFF)) {
		m_ram[address - 0xA000] = value;
	} else {
		std::stringstream stream;
		stream << "Cartridge[NoMBC]: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
}

void NoMBC::reset() {
	Cartridge::reset();

	std::memset(m_ram, 0, sizeof(m_ram));
}
}; // namespace Zirc
