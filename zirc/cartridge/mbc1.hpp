#pragma once

#include <cstdint>
#include <cstring>

#include "cartridge.hpp"

namespace Zirc {
class MBC1 : public Cartridge {
  public:
	MBC1(const std::vector<uint8_t> cartridgeData);

	uint8_t read8(const uint16_t address) const override;
	void write8(const uint16_t address, const uint8_t value) override;
	void reset() override;

  private:
	static constexpr size_t ROM_BANK_SIZE = (1 << 14);
	static constexpr size_t RAM_BANK_SIZE = (1 << 13);

	std::vector<std::array<uint8_t, ROM_BANK_SIZE>> m_romBanks;
	std::vector<std::array<uint8_t, RAM_BANK_SIZE>> m_ramBanks;

	bool m_bankingMode = false;

	uint8_t m_bankNumber = 0x1;
	uint8_t m_romExtraBankNumber = 0;

	bool m_hasRam = false;
	uint8_t m_ramBankNumber = 0;
	bool m_ramEnabled = false;
};
} // namespace Zirc
