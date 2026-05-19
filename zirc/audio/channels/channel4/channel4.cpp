#include <iomanip>
#include <sstream>

#include "lfsr.hpp"
#include "channel4.hpp"

namespace Zirc {
void Channel4::tick(const float sampleRate) {
	double ticksPerSample = static_cast<double>(getTickFrequency()) / sampleRate;

	m_tickSampleAcc += ticksPerSample;
	while(m_tickSampleAcc >= 1.0) {
		m_tickSampleAcc -= 1.0;
		m_lfsr.tick();
	}
}

void Channel4::trigger(void) {
	turnOn();
	m_envelopeAcc = 0;
	m_volume = getInitialVolume();
	m_lfsr.reset();
	m_tickSampleAcc = 0;

	if(m_lengthTimer == 0) { m_lengthTimer = 63 - (m_nr41 & 0x3F); }
}

void Channel4::reset() {
	m_isOn = false;

	m_nr41 = 0;
	m_nr42 = 0;
	m_nr43 = 0;
	m_nr44 = 0;

	m_lengthTimer = 0;

	m_envelopeAcc = 0;
	m_volume = 15;

	m_lfsr.reset();

	m_tickSampleAcc = 0.0;
}

void Channel4::write8(const uint16_t address, const uint8_t value) {
	switch(address) {
	case 0xFF20: m_nr41 = value; break;
	case 0xFF21: m_nr42 = value; break;
	case 0xFF22:
		m_nr43 = value;
		if(getLfsrWidth() == 0) {
			m_lfsr.setLongMode();
		} else {
			m_lfsr.setShortMode();
		}
		break;
	case 0xFF23:
		m_nr44 = value & 0x7F;
		if((value & 0x80) != 0) { trigger(); }
		break;
	default: {
		std::stringstream stream;
		stream << "Audio[CH4]: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

uint8_t Channel4::read8(const uint16_t address) const {
	switch(address) {
	case 0xFF20: return 0xFF;
	case 0xFF21: return m_nr42;
	case 0xFF22: return m_nr43;
	case 0xFF23: return m_nr44 & 0x40;
	default: {
		std::stringstream stream;
		stream << "Audio[CH4]: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
			   << uint(address) << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

void Channel4::doEventEnvelopeSweep() {
	if(getSweepPace() != 0) {
		m_envelopeAcc++;
		if(m_envelopeAcc >= getSweepPace()) {
			m_envelopeAcc = 0;

			if((m_nr42 & 0x8) == 0 && m_volume > 0) {
				m_volume--;
			} else if((m_nr42 & 0x8) != 0 && m_volume < 15) {
				m_volume++;
			}
		}
	}
}

void Channel4::doEventSoundLength() {
	if(m_isOn) {
		if(m_lengthTimer > 0) { m_lengthTimer--; }
		if((m_nr44 & 0x40) != 0 && m_lengthTimer == 0) { turnOff(); }
	}
}
} // namespace Zirc
