#include <iomanip>
#include <iostream>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "mbc1.hpp"
#include "no_mbc.hpp"

namespace Zirc {
#define CARTRIDGE_NINTENDO_LOGO_OFFSET 0x104
#define CARTRIDGE_NINTENDO_LOGO_LEN 48

#define CARTRIDGE_TITLE_OFFSET 0x134
#define CARTRIDGE_TITLE_LEN 16

#define CARTRIDGE_MANUFRACTURER_CODE_OFFSET 0x13F
#define CARTRIDGE_MANUFRACTURER_CODE_LEN 4

#define CARTRIDGE_CGB_FLAG_OFFSET 0x143

#define CARTRIDGE_NEW_LICENSEE_CODE_OFFSET 0x144
#define CARTRIDGE_NEW_LICENSEE_CODE_LEN 2

#define CARTRIDGE_SGB_FLAG_OFFSET 0x146

#define CARTRIDGE_TYPE_OFFSET 0x147

// clang-format off
const std::unordered_map<uint8_t, const char *> CARTRIDGE_TYPES = {
	{0x00,                       "ROM ONLY"},
	{0x01,                           "MBC1"},
	{0x02,                       "MBC1+RAM"},
	{0x03,               "MBC1+RAM+BATTERY"},
	{0x05,                           "MBC2"},
	{0x06,                   "MBC2+BATTERY"},
	{0x08,                        "ROM+RAM"},
	{0x09,                "ROM+RAM+BATTERY"},
	{0x0B,                          "MMM01"},
	{0x0C,                      "MMM01+RAM"},
	{0x0D,              "MMM01+RAM+BATTERY"},
	{0x0F,             "MBC3+TIMER+BATTERY"},
	{0x10,         "MBC3+TIMER+RAM+BATTERY"},
	{0x11,                           "MBC3"},
	{0x12,                       "MBC3+RAM"},
	{0x13,               "MBC3+RAM+BATTERY"},
	{0x19,                           "MBC5"},
	{0x1A,                       "MBC5+RAM"},
	{0x1B,               "MBC5+RAM+BATTERY"},
	{0x1C,                    "MBC5+RUMBLE"},
	{0x1D,                "MBC5+RUMBLE+RAM"},
	{0x1E,        "MBC5+RUMBLE+RAM+BATTERY"},
	{0x20,                           "MBC6"},
	{0x22, "MBC7+SENSOR+RUMBLE+RAM+BATTERY"},
	{0xFC,                  "POCKET CAMERA"},
	{0xFD,                   "BANDAI TAMA5"},
	{0xFE,                           "HuC3"},
	{0xFF,               "HuC1+RAM+BATTERY"}
};

#define CARTRIDGE_ROM_TYPE_OFFSET 0x148

const std::unordered_map<Cartridge::RomType, const char *> ROM_TYPES = {
	{Cartridge::RomType::Banks2,   "32 KiB [2 ROM banks (no-baking)]"},
	{Cartridge::RomType::Banks4,               "64 KiB [4 ROM banks]"},
	{Cartridge::RomType::Banks8,              "128 KiB [8 ROM banks]"},
	{Cartridge::RomType::Banks16,            "256 KiB [16 ROM banks]"},
	{Cartridge::RomType::Banks32,            "512 KiB [32 ROM banks]"},
	{Cartridge::RomType::Banks64,              "1 MiB [64 ROM banks]"},
	{Cartridge::RomType::Banks128,            "2 MiB [128 ROM banks]"},
	{Cartridge::RomType::Banks256,            "4 MiB [256 ROM banks]"},
	{Cartridge::RomType::Banks512,            "8 MiB [512 ROM banks]"},
	{Cartridge::RomType::Banks72,                "1.1 MiB [72 banks]"},
	{Cartridge::RomType::Banks80,                "1.2 MiB [80 banks]"},
	{Cartridge::RomType::Banks96,                "1.5 MiB [96 banks]"}
};

#define CARTRIDGE_RAM_TYPE_OFFSET 0x149

const std::unordered_map<Cartridge::RamType, const char *> RAM_TYPES = {
	{Cartridge::RamType::NoRam,            "0 [No RAM]"},
	{Cartridge::RamType::Banks1,       "8 KiB [1 bank]"},
	{Cartridge::RamType::Banks4,     "32 KiB [4 banks]"},
	{Cartridge::RamType::Banks16,  "128 KiB [16 banks]"},
	{Cartridge::RamType::Banks8,     "64 KiB [8 banks]"}
};
// clang-format on

#define CARTRIDGE_DESTINATION_CODE_OFFSET 0x14A

#define CARTRIDGE_OLD_LICENSEE_CODE_OFFSET 0x14B

#define CARTRIDGE_ROM_VERSION_OFFSET 0x14C

#define CARTRIDGE_HEADER_CHECKSUM_OFFSET 0x14D

Cartridge::Cartridge(const std::vector<uint8_t> fileData) {
	m_title =
		std::string(reinterpret_cast<const char *>(fileData.data()) + CARTRIDGE_TITLE_OFFSET, CARTRIDGE_TITLE_LEN);
	m_romVersionNumber = fileData[CARTRIDGE_ROM_VERSION_OFFSET];

	m_type = fileData[CARTRIDGE_TYPE_OFFSET];

	const uint8_t romType = fileData[CARTRIDGE_ROM_TYPE_OFFSET];
	if(!((romType <= 0x08) || (romType >= 0x52 && romType <= 0x54))) {
		std::stringstream stream;
		stream << "Cartridge: Invalid RomType with value 0x" << std::hex << std::setw(2) << std::setfill('0')
			   << uint(romType) << std::endl;
		throw std::runtime_error(stream.str());
	}
	m_romType = static_cast<RomType>(romType);

	const uint8_t ramType = fileData[CARTRIDGE_RAM_TYPE_OFFSET];
	assert(ramType != 0x01 && "Cartridge: Invalid ram type 0x01.\n");
	if(ramType > 0x05) {
		std::stringstream stream;
		stream << "Cartridge: Invalid RamType with value 0x" << std::hex << std::setw(2) << std::setfill('0')
			   << uint(ramType) << std::endl;
		throw std::runtime_error(stream.str());
	}
	m_ramType = static_cast<RamType>(ramType);

	m_destinationCode = fileData[CARTRIDGE_DESTINATION_CODE_OFFSET];
	m_manufacturerCode =
		std::vector<uint8_t>(fileData.data() + CARTRIDGE_MANUFRACTURER_CODE_OFFSET,
							 fileData.data() + CARTRIDGE_MANUFRACTURER_CODE_OFFSET + CARTRIDGE_MANUFRACTURER_CODE_LEN);

	m_oldLicenseeCode = fileData[CARTRIDGE_OLD_LICENSEE_CODE_OFFSET];
	m_newLicenseeCode = static_cast<uint16_t>((fileData[CARTRIDGE_NEW_LICENSEE_CODE_OFFSET] << 8) |
											  fileData[CARTRIDGE_NEW_LICENSEE_CODE_OFFSET + 1]);

	m_cgbFlag = fileData[CARTRIDGE_CGB_FLAG_OFFSET];
	m_sgbFlag = fileData[CARTRIDGE_SGB_FLAG_OFFSET];

	m_bootRomMapped = true;

	assert(Cartridge::calculateHeaderChecksum(fileData) == fileData[CARTRIDGE_HEADER_CHECKSUM_OFFSET] &&
		   "Cartridge: Header checksum is not valid.");
}

std::unique_ptr<Cartridge> Cartridge::createCartridge(const std::string &path) {
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if(!file) { throw std::runtime_error("Could not open the cartridge file.\n"); }

	std::streamsize size = file.tellg();
	if(size < 0) { throw std::runtime_error("Failed to read the size of the cartridge file.\n"); }
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> fileData(static_cast<size_t>(size));

	if(!file.read(reinterpret_cast<char *>(fileData.data()), size)) {
		throw std::runtime_error("Failed to read from the cartridge file.\n");
	}

	uint8_t type = fileData[CARTRIDGE_TYPE_OFFSET];

	switch(type) {
	case 0x00: return std::make_unique<NoMBC>(fileData); break;
	case 0x01:
	case 0x02:
	case 0x03: return std::make_unique<MBC1>(fileData); break;
	default:
		std::stringstream stream;
		stream << "Cartridge: Unimplemented cartridge of type \""
			   << (CARTRIDGE_TYPES.find(type) != CARTRIDGE_TYPES.end() ? CARTRIDGE_TYPES.at(type) : "Unknown")
			   << "\" (0x" << std::hex << std::setw(2) << std::setfill('0') << uint(type) << ")" << std::endl;
		throw std::runtime_error(stream.str());
	}
}

void Cartridge::debug(void) const {
	std::cout << "== ROM DEBUG ==" << std::endl;
	std::cout << "- Title: \"" << m_title << "\"" << std::endl;
	std::cout << "- Type: "
			  << (CARTRIDGE_TYPES.find(m_type) != CARTRIDGE_TYPES.end() ? CARTRIDGE_TYPES.at(m_type) : "Unknown")
			  << " (0x" << std::hex << std::setw(2) << std::setfill('0') << uint(m_type) << ")" << std::endl;
	std::cout << "- ROM size: " << (ROM_TYPES.find(m_romType) != ROM_TYPES.end() ? ROM_TYPES.at(m_romType) : "Unknown")
			  << " (0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint>(m_romType) << ")"
			  << std::endl;
	std::cout << "- RAM size: " << (RAM_TYPES.find(m_ramType) != RAM_TYPES.end() ? RAM_TYPES.at(m_ramType) : "Unknown")
			  << " (0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint>(m_ramType) << ")"
			  << std::endl;
	std::cout << "- ROM version: 0x" << std::hex << std::setw(2) << std::setfill('0') << uint(m_romVersionNumber)
			  << std::endl;
	std::cout << "- Destination code: 0x" << std::hex << std::setw(2) << std::setfill('0') << uint(m_destinationCode)
			  << std::endl;
	std::cout << "- Licensee code (" << (m_oldLicenseeCode == 0x33 ? "new" : "old") << "): 0x" << std::hex
			  << std::setw(2) << std::setfill('0')
			  << int(m_oldLicenseeCode == 0x33 ? m_newLicenseeCode : m_oldLicenseeCode) << std::endl;
}

// clang-format off
const std::array<uint8_t, 256> BOOT_ROM = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x00, 0x00, 0x0C, 0x3E, 0xF3, 0x00, 0x00, 0x3E, 0x77, 0x00, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0xA8, 0x00, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0x00, 0x0C, 0x3E, 0x87, 0x00, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x0E, 0xCC, 0x0D, 0x00, 0x0B, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x67, 0x68, 0x0E, 0xDD, 0xDD, 0xC8, 0x88, 0x66, 0x63,
    0x60, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x21, 0xA8, 0x00, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x00,
    0x00, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50
};
// clang-format on
uint8_t Cartridge::read8BootRom(const uint16_t address) const {
	return m_usingCustomBootRom ? m_customBootRom[address] : BOOT_ROM[address];
}
} // namespace Zirc
