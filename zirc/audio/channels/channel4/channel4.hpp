#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>

#include "lfsr.hpp"

namespace Zirc {
class Channel4 {
  public:
	Channel4() { reset(); }

	void tick(const float sampleRate);
	void trigger();
	void reset();

	void write8(const uint16_t address, const uint8_t value);
	uint8_t read8(const uint16_t address) const;

	void doEventEnvelopeSweep();
	void doEventSoundLength();

	inline float getSample(const float amplitude) {
		if(!m_isOn) { return 0.0f; }
		return (m_lfsr.getBit() ? -1.0f : 1.0f) * amplitude * m_volume / 15.0f;
	}

	inline bool isOn() const { return m_isOn; }
	inline bool isDacOn() const { return (m_nr42 & 0xF8) != 0; }

  private:
	inline void turnOff() { m_isOn = false; }
	inline void turnOn() { m_isOn = true; }

	inline double getTickFrequency() const {
		double clockDivider = getClockDivider();
		if(clockDivider == 0) { clockDivider = 0.5; }
		return 262144.0 / (clockDivider * powf(2, getClockShift()));
	}

	inline uint8_t getSweepPace() const { return m_nr42 & 0x7; }
	inline uint8_t getInitialVolume() const { return (m_nr42 & 0xF0) >> 4; }

	inline uint8_t getClockShift() const { return (m_nr43 >> 4) & 0xF; }
	inline uint8_t getLfsrWidth() const { return (m_nr43 >> 3) & 0x1; }
	inline uint8_t getClockDivider() const { return m_nr43 & 0x7; }

	bool m_isOn;

	Lfsr m_lfsr;
	uint8_t m_volume;

	uint8_t m_nr41, m_nr42, m_nr43, m_nr44;

	uint8_t m_lengthTimer;

	int m_envelopeAcc;
	double m_tickSampleAcc;
};
} // namespace Zirc
