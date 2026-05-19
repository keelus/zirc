#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "../common.hpp"
#include "cartridge.hpp"
#include "mbc1.hpp"

namespace Zirc {
using RomType = Cartridge::RomType;
using RamType = Cartridge::RamType;

// clang-format off
const std::unordered_map<RomType, uint16_t> ROM_BANK_AMOUNT = {
    {RomType::Banks2,     2},
    {RomType::Banks4,     4},
    {RomType::Banks8,     8},
    {RomType::Banks16,   16},
    {RomType::Banks32,   32},
    {RomType::Banks64,   64},
    {RomType::Banks128, 128},
    {RomType::Banks256, 256},
    {RomType::Banks512, 512},
    {RomType::Banks72,   72},
    {RomType::Banks80,   80},
    {RomType::Banks96,   96}
};

const std::unordered_map<RomType, uint16_t> ROM_BANK_NUMBER_MASK = {
    {RomType::Banks2,   (1 << 1) - 1 },
    {RomType::Banks4,   (1 << 2) - 1 },
    {RomType::Banks8,   (1 << 3) - 1 },
    {RomType::Banks16,  (1 << 4) - 1 },
    {RomType::Banks32,  (1 << 5) - 1 },
    {RomType::Banks64,  (1 << 6) - 1 },
    {RomType::Banks128, (1 << 7) - 1 },
    {RomType::Banks256, (1 << 8) - 1 },
    {RomType::Banks512, (1 << 9) - 1 },
    {RomType::Banks72,  (1 << 7) - 1 },
    {RomType::Banks80,  (1 << 7) - 1 },
    {RomType::Banks96,  (1 << 7) - 1 }
};

const std::unordered_map<RamType, uint8_t> RAM_BANK_AMOUNT = {
    {RamType::NoRam,    0},
    {RamType::Banks1,   1},
    {RamType::Banks4,   4},
    {RamType::Banks16, 16},
    {RamType::Banks8,   8}
};
// clang-format on

#define ROM_BANK_SIZE (1 << 14)
#define RAM_BANK_SIZE (1 << 13)

MBC1::MBC1(const std::vector<uint8_t> cartridgeData) : Cartridge(cartridgeData) {
	uint16_t romBankAmount = ROM_BANK_AMOUNT.at(romType());
	uint8_t ramBankAmount = RAM_BANK_AMOUNT.at(ramType());

	if(romBankAmount >= 64) {
		throw std::runtime_error("Cartridge[MBC1]: 1MiB or more cartridges are not supported yet.");
	}

	m_romBanks.resize(romBankAmount);
	for(size_t i = 0; i < romBankAmount; i++) {
		std::memset(m_romBanks[i].data(), 0, sizeof(uint8_t) * ROM_BANK_SIZE);
	}

	assert(cartridgeData.size() == romBankAmount * ROM_BANK_SIZE);
	for(size_t romBankIndex = 0; romBankIndex < romBankAmount; romBankIndex++) {
		std::memcpy(m_romBanks[romBankIndex].data(), cartridgeData.data() + romBankIndex * ROM_BANK_SIZE,
					sizeof(uint8_t) * ROM_BANK_SIZE);
	}

	m_hasRam = type() == 0x02 || type() == 0x03;
	if(m_hasRam) {
		assert(ramBankAmount > 0 && "Cartridge[MBC1]: Cartridge type indicates RAM, but RAM type is 0.\n");

		m_ramBanks.resize(ramBankAmount);
		for(size_t ramBankIndex = 0; ramBankIndex < ramBankAmount; ramBankIndex++) {
			std::memset(m_ramBanks[ramBankIndex].data(), 0, sizeof(uint8_t) * RAM_BANK_SIZE);
		}
	}
}

uint8_t MBC1::read8(const uint16_t address) const {
	if(IN_RANGE(address, 0x0000, 0x3FFF)) {
		if(isBootRomMapped() && address < 0x100) { return read8BootRom(address); }
		return m_romBanks.at(0).at(address);
	}

	if(IN_RANGE(address, 0x4000, 0x7FFF)) {
		uint16_t bankNumber = m_bankNumber;
		if(bankNumber == 0x0) { bankNumber = 0x1; }

		uint16_t finalBank = bankNumber & ROM_BANK_NUMBER_MASK.at(romType());
		return m_romBanks.at(finalBank)[address - 0x4000];
	}

	if(IN_RANGE(address, 0xA000, 0xBFFF)) {
		if(m_ramEnabled) {
			if(m_bankingMode == 0) {
				return m_ramBanks.at(0)[address - 0xA000];
			} else {
				return m_ramBanks.at(m_ramBankNumber)[address - 0xA000];
			}
		} else {
			return 0xFF;
		}
	}

	std::stringstream stream;
	stream << "Cartridge[MBC1]: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
		   << uint(address) << std::endl;
	throw std::runtime_error(stream.str());
}

void MBC1::write8(const uint16_t address, const uint8_t value) {
	if(IN_RANGE(address, 0x0000, 0x1FFF)) {
		m_ramEnabled = (value & 0xF) == 0xA;
		return;
	}

	if(IN_RANGE(address, 0x2000, 0x3FFF)) {
		m_bankNumber = value & 0x1F;
		return;
	}

	if(IN_RANGE(address, 0x4000, 0x5FFF)) {
		uint8_t bankNumber = value & 0x3;
		if(bankNumber < RAM_BANK_AMOUNT.at(ramType())) { m_ramBankNumber = bankNumber; }
		return;
	}

	if(IN_RANGE(address, 0x6000, 0x7FFF)) {
		m_bankingMode = value & 0x1;
		return;
	}

	if(IN_RANGE(address, 0xA000, 0xBFFF)) {
		if(m_ramEnabled) {
			if(m_bankingMode == 0) {
				m_ramBanks.at(0)[address - 0xA000] = value;
			} else {
				m_ramBanks.at(m_ramBankNumber)[address - 0xA000] = value;
			}
		}
		return;
	}

	std::stringstream stream;
	stream << "Cartridge[MBC1]: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
		   << uint(address) << std::endl;
	throw std::runtime_error(stream.str());
}

void MBC1::reset() {
	Cartridge::reset();

	m_bankingMode = false;
	m_bankNumber = 0x1;
	m_romExtraBankNumber = 0;

	m_ramBankNumber = 0;
	m_ramEnabled = false;

	if(m_hasRam) {
		for(size_t i = 0; i < m_ramBanks.size(); i++) {
			memset(m_ramBanks[i].data(), 0, sizeof(uint8_t) * RAM_BANK_SIZE);
		}
	}
}
} // namespace Zirc
