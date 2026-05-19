#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>

namespace Zirc {
class Channel1 {
  public:
	Channel1() { reset(); }

	void tick(const float sampleRate);
	void trigger();
	void reset();

	void write8(const uint16_t address, const uint8_t value);
	uint8_t read8(const uint16_t address) const;

	void doEventEnvelopeSweep();
	void doEventSoundLength();
	void doEventFrequencySweep();

	inline float getSample(const float amplitude) {
		if(!isOn()) { return 0; }

		uint8_t duty = (m_nr11 >> 6) & 0x3;
		double dutyPercent = 0;
		switch(duty) {
		case 0b00: dutyPercent = 12.5f; break;
		case 0b01: dutyPercent = 25.0f; break;
		case 0b10: dutyPercent = 50.0f; break;
		case 0b11: dutyPercent = 75.0f; break;
		}
		assert(dutyPercent != 0);

		double phaseThreshold = (2 * M_PI) * (dutyPercent / 100.0f);
		float value = amplitude * static_cast<float>(m_volume) / 15.0f;
		return (m_phase < phaseThreshold ? 1 : -1) * value;
	}

	inline bool isOn() const { return m_isOn; }
	inline bool isDacOn() const { return (m_nr12 & 0xF8) != 0; }

  private:
	inline void turnOff() { m_isOn = false; }
	inline void turnOn() { m_isOn = true; }

	inline uint16_t getPeriod() const { return static_cast<uint16_t>(((m_nr14 & 0x7) << 8) | m_nr13); }
	inline void setPeriod(const uint16_t value) {
		m_nr13 = static_cast<uint8_t>(value & 0xFF);
		m_nr14 = static_cast<uint8_t>((m_nr14 & 0xF8) | (value >> 8));
	}

	inline double getFrequency() const {
		int base_freq = 131072 / (2048 - getPeriod());
		return base_freq;
	}

	inline uint8_t getSweepPace() const { return m_nr12 & 0x7; }
	inline uint8_t getPace() const { return (m_nr10 & 0x70) >> 5; }
	inline uint8_t getIndividualStep() const { return m_nr10 & 0x7; }
	inline uint8_t getPeriodDirection() const { return (m_nr10 >> 3) & 0x1; }
	inline uint8_t getInitialVolume() const { return (m_nr12 & 0xF0) >> 4; }

	bool m_isOn;
	bool m_enabledFlag;

	double m_phase;
	uint8_t m_volume;

	uint8_t m_nr10, m_nr11, m_nr12, m_nr13, m_nr14;

	uint8_t m_lengthTimer;
	uint16_t m_periodDivider;

	int m_envelopeAcc;
	int m_periodSweepAcc;
};
} // namespace Zirc
