#include <iomanip>
#include <sstream>

#include "channel2.hpp"

namespace Zirc {
void Channel2::tick(const float sampleRate) {
	m_periodDivider++;
	if(m_periodDivider == 0x800) { m_periodDivider = getPeriod(); }

	double freq = getFrequency();
	if(freq > sampleRate / 2.0) { freq = sampleRate / 2.0; }

	m_phase += 2.0 * M_PI * freq / sampleRate;
	if(m_phase >= 2.0 * M_PI) { m_phase -= 2.0 * M_PI; }
}

void Channel2::trigger(void) {
	turnOn();
	m_envelopeAcc = 0;
	m_volume = getInitialVolume();

	if(m_lengthTimer == 0) { m_lengthTimer = 63 - (m_nr21 & 0x3F); }
}

void Channel2::reset() {
	m_phase = 0.0;

	m_isOn = false;

	m_nr21 = 0;
	m_nr22 = 0;
	m_nr23 = 0;
	m_nr24 = 0;

	m_lengthTimer = 0;

	m_periodDivider = 0;

	m_envelopeAcc = 0;
	m_volume = 15;
}

void Channel2::write8(const uint16_t address, const uint8_t value) {
	switch(address) {
	case 0xFF16: m_nr21 = value; break;
	case 0xFF17: m_nr22 = value; break;
	case 0xFF18: m_nr23 = value; break;
	case 0xFF19:
		m_nr24 = value & 0x7F;
		if((value & 0x80) != 0) { trigger(); }
		break;

	default: {
		std::stringstream stream;
		stream << "Audio[CH2]: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

uint8_t Channel2::read8(const uint16_t address) const {
	switch(address) {
	case 0xFF16: return m_nr21 & 0xC0;
	case 0xFF17: return m_nr22;
	case 0xFF18: return 0xFF;
	case 0xFF19: return m_nr24 & 0x40;
	default: {
		std::stringstream stream;
		stream << "Audio[CH2]: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

void Channel2::doEventEnvelopeSweep() {
	if(getSweepPace() != 0) {
		m_envelopeAcc++;
		if(m_envelopeAcc >= getSweepPace()) {
			m_envelopeAcc = 0;

			if((m_nr22 & 0x8) == 0 && m_volume > 0) {
				m_volume--;
			} else if((m_nr22 & 0x8) != 0 && m_volume < 15) {
				m_volume++;
			}
		}
	}
}

void Channel2::doEventSoundLength() {
	if(m_isOn) {
		if(m_lengthTimer > 0) { m_lengthTimer--; }
		if((m_nr24 & 0x40) != 0 && m_lengthTimer == 0) { turnOff(); }
	}
}
} // namespace Zirc
