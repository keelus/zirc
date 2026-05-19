#pragma once

#include <cstdint>
#include <cstring>

namespace Zirc {
#define WORK_RAM_SIZE (1 << 13)
#define HIGH_RAM_SIZE (1 << 7)

/*
 * 0xC000 - 0xCFFF: Work RAM (4 KiB)
 * 0xD000 - 0xD000: Work RAM (4 KiB)
 * 0xE000 - 0xFDFF: Echo RAM
 * 0xFF80 - 0xFFFE: High RAM (127 B)
 */
class Memory {
  public:
	Memory() { reset(); }

	uint8_t read8(const uint16_t address) const;
	uint16_t read16(const uint16_t address) const;

	void write8(const uint16_t address, const uint8_t value);
	void write16(const uint16_t address, const uint16_t value);

	inline void reset() {
		std::memset(m_wram, 0, sizeof(m_wram));
		std::memset(m_hram, 0, sizeof(m_hram));
	}

  private:
	uint8_t m_wram[WORK_RAM_SIZE];
	uint8_t m_hram[HIGH_RAM_SIZE];
};
} // namespace Zirc
