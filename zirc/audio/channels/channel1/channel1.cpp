#include <cmath>
#include <iomanip>
#include <sstream>

#include "channel1.hpp"

namespace Zirc {
void Channel1::tick(const float sampleRate) {
	m_periodDivider++;
	if(m_periodDivider == 0x800) { m_periodDivider = getPeriod(); }

	if(!m_isOn) { return; }

	double freq = getFrequency();
	if(freq > static_cast<double>(sampleRate) / 2.0) { freq = static_cast<double>(sampleRate) / 2.0; }

	m_phase += 2.0 * M_PI * freq / static_cast<double>(sampleRate);
	if(m_phase >= 2.0 * M_PI) { m_phase -= 2.0 * M_PI; }
}

void Channel1::trigger(void) {
	turnOn();
	m_envelopeAcc = 0;
	m_volume = getInitialVolume();
	m_periodSweepAcc = 0;
	m_enabledFlag = getSweepPace() != 0 || getIndividualStep() != 0;

	if(m_lengthTimer == 0) { m_lengthTimer = 63 - (m_nr11 & 0x3F); }
}

void Channel1::reset() {
	m_isOn = false;
	m_enabledFlag = false;

	m_phase = 0.0;
	m_volume = 15;

	m_nr10 = 0, m_nr11 = 0, m_nr12 = 0, m_nr13 = 0, m_nr14 = 0;

	m_lengthTimer = 0;
	m_periodDivider = 0;

	m_envelopeAcc = 0;
	m_periodSweepAcc = 0;
}

void Channel1::write8(const uint16_t address, const uint8_t value) {
	switch(address) {
	case 0xFF10: m_nr10 = value & 0x7F; break;
	case 0xFF11: m_nr11 = value; break;
	case 0xFF12: m_nr12 = value; break;
	case 0xFF13: m_nr13 = value; break;
	case 0xFF14:
		m_nr14 = value & 0x7F;
		if((value & 0x80) != 0) { trigger(); }
		break;

	default: {
		std::stringstream stream;
		stream << "Audio: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0') << uint(address)
			   << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

uint8_t Channel1::read8(const uint16_t address) const {
	switch(address) {
	case 0xFF10: return m_nr10;
	case 0xFF11: return m_nr11 & 0xC0;
	case 0xFF12: return m_nr12;
	case 0xFF13: return 0xFF;
	case 0xFF14: return m_nr14 & 0x40;
	default: {
		std::stringstream stream;
		stream << "Audio: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0') << uint(address)
			   << std::endl;
		throw std::runtime_error(stream.str());
	}
	}
}

void Channel1::doEventEnvelopeSweep() {
	if(m_enabledFlag && getSweepPace() != 0) {
		m_envelopeAcc++;
		if(m_envelopeAcc >= getSweepPace()) {
			m_envelopeAcc = 0;

			if((m_nr12 & 0x8) == 0 && m_volume > 0) {
				m_volume--;
			} else if((m_nr12 & 0x8) != 0 && m_volume < 15) {
				m_volume++;
			}
		}
	}
}

void Channel1::doEventSoundLength() {
	if(m_isOn) {
		if(m_lengthTimer > 0) { m_lengthTimer--; }
		if((m_nr14 & 0x40) != 0 && m_lengthTimer == 0) { turnOff(); }
	}
}

void Channel1::doEventFrequencySweep() {
	if(m_enabledFlag && getPace() != 0) {
		m_periodSweepAcc++;
		if(m_periodSweepAcc > getPace()) {
			m_periodSweepAcc = 0;
			uint16_t finalStep = getPeriod() / static_cast<uint16_t>(powf(2, getIndividualStep()));
			bool overflows = false;
			if(getPeriodDirection() == 0 && getPeriod() < 0x7FF) {
				uint16_t newPeriod = getPeriod() + finalStep;
				if(newPeriod > 0x7FF) {
					overflows = true;
				} else {
					setPeriod(newPeriod);
				}
			} else if(getPeriodDirection() != 0 && getPeriod() > 0) {
				uint16_t newPeriod = (getPeriod() - finalStep);
				if(newPeriod > 0x7FF) {
					overflows = true;
				} else {
					setPeriod(newPeriod);
				}
			}

			if(overflows) { turnOff(); }
		}
	}
}
} // namespace Zirc
