#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#include "cartridge.hpp"

namespace Zirc {
class NoMBC : public Cartridge {
  public:
	NoMBC(const std::vector<uint8_t> cartridgeData) : Cartridge(cartridgeData) {
		assert(type() == 0x00);
		std::memcpy(m_rom, cartridgeData.data(), sizeof(uint8_t) * cartridgeData.size());
	}

	uint8_t read8(const uint16_t address) const override;
	void write8(const uint16_t address, const uint8_t value) override;
	void reset() override;

  private:
	static constexpr size_t CARTRIDGE_ROM_SIZE = (1 << 15);
	static constexpr size_t CARTRIDGE_RAM_SIZE = (1 << 13);

	uint8_t m_rom[CARTRIDGE_ROM_SIZE];
	uint8_t m_ram[CARTRIDGE_RAM_SIZE];
};
} // namespace Zirc
