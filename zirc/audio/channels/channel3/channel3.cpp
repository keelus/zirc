#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "channel3.hpp"

namespace Zirc {
void Channel3::tick(float sampleRate) {
	double freq = getFrequency();
	if(freq > sampleRate / 2.0) freq = sampleRate / 2.0;

	double step = (freq * 32.0) / sampleRate;

	m_phase += step;
	while(m_phase >= 1.0) {
		m_phase -= 1.0;
		m_waveIndex = (m_waveIndex + 1) & 31;
	}
}

void Channel3::trigger(void) {
	turnOn();
	m_waveIndex = 0;

	if(m_lengthTimer == 0) { m_lengthTimer = 256; }
}

void Channel3::reset() {
	m_phase = 0.0;

	m_isOn = false;

	m_nr30 = 0;
	m_nr31 = 0;
	m_nr32 = 0;

	m_lengthEnabled = false;

	m_periodDivider = 0;

	std::memset(m_wave, 0, sizeof(m_wave));
	m_waveIndex = 0;

	m_lengthTimer = 0;
}

void Channel3::write8(const uint16_t address, const uint8_t value) {
	switch(address) {
	case 0xFF1A:
		m_nr30 = value & 0x80;
		if(!isDacOn()) turnOff();
		break;
	case 0xFF1B:
		m_nr31 = value;
		m_lengthTimer = 256 - value;
		break;
	case 0xFF1C: m_nr32 = value & 0x60; break;
	case 0xFF1D: m_periodDivider = (m_periodDivider & 0x700) | static_cast<uint16_t>(value); break;
	case 0xFF1E:
		if(value & 0x80) { trigger(); }
		m_periodDivider = (m_periodDivider & 0xFF) | ((static_cast<uint16_t>(value) << 8) & 0x700);
		m_lengthEnabled = (value & 0x40) != 0;
		break;
	default: {
		if(address >= 0xFF30 && address <= 0xFF3F) {
			m_wave[address - 0xFF30] = value;
			return;
		}

		std::stringstream stream;
		stream << "Audio[CH2]: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

uint8_t Channel3::read8(const uint16_t address) const {
	switch(address) {
	case 0xFF1A: return m_nr30;
	case 0xFF1C: return m_nr32;
	case 0xFF1E: return static_cast<uint8_t>(m_lengthEnabled << 6); break;
	default: {
		if(address >= 0xFF30 && address <= 0xFF3F) { return m_wave[address - 0xFF30]; }

		std::stringstream stream;
		stream << "Audio[CH2]: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

void Channel3::doEventSoundLength() {
	if(m_isOn && m_lengthEnabled) {
		if(m_lengthTimer > 0) { m_lengthTimer--; }
		if(m_lengthTimer == 0) { turnOff(); }
	}
}
} // namespace Zirc
