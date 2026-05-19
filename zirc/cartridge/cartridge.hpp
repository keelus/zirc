#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace Zirc {
class Cartridge {
  public:
	enum class RomType {
		Banks2 = 0x00,
		Banks4,
		Banks8,
		Banks16,
		Banks32,
		Banks64,
		Banks128,
		Banks256,
		Banks512,

		/* Unused */
		Banks72 = 0x52,
		Banks80,
		Banks96,
	};

	enum class RamType {
		NoRam = 0x00,
		Banks1 = 0x02,
		Banks4,
		Banks16,
		Banks8,
	};

	Cartridge(const std::vector<uint8_t> fileData);
	virtual ~Cartridge() = default;

	static std::unique_ptr<Cartridge> createCartridge(const std::string &path);
	void debug(void) const;

	inline uint8_t type() const { return m_type; }

	inline RomType romType() const { return m_romType; }
	inline RamType ramType() const { return m_ramType; }

	inline std::vector<uint8_t> manufracturerCode() const { return m_manufacturerCode; }

	inline void unmapBootRom() { m_bootRomMapped = false; }
	inline bool isBootRomMapped() const { return m_bootRomMapped; }

	inline void setCustomBootRom(const uint8_t *customBootRom) {
		m_usingCustomBootRom = true;
		std::memcpy(m_customBootRom.data(), customBootRom, sizeof(uint8_t) * 256);
	}
	inline void disableCustomBootRom() { m_usingCustomBootRom = false; }

	virtual uint8_t read8(const uint16_t address) const = 0;
	virtual void write8(const uint16_t address, const uint8_t value) = 0;
	virtual void reset() { m_bootRomMapped = true; }

  protected:
	uint8_t read8BootRom(const uint16_t address) const;

  private:
	static uint8_t calculateHeaderChecksum(const std::vector<uint8_t> cartridgeData) {
		uint8_t checksum = 0;
		for(uint16_t index = 0x134; index <= 0x14C; index++) {
			checksum = static_cast<uint8_t>(checksum - cartridgeData[index] - 1);
		}

		return checksum;
	}

	std::string m_title;
	uint8_t m_romVersionNumber;

	uint8_t m_type;
	RomType m_romType;
	RamType m_ramType;

	uint8_t m_destinationCode;
	std::vector<uint8_t> m_manufacturerCode;

	uint8_t m_oldLicenseeCode;
	uint16_t m_newLicenseeCode;

	bool m_cgbFlag;
	bool m_sgbFlag;

	bool m_usingCustomBootRom = false;
	std::array<uint8_t, 256> m_customBootRom;
	bool m_bootRomMapped;
};
} // namespace Zirc
