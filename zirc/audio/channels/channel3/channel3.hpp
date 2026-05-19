#pragma once

#include <cassert>
#include <cstdint>

namespace Zirc {
class Channel3 {
  public:
	Channel3() { reset(); }

	void tick(const float sampleRate);
	void trigger();
	void reset();

	void write8(const uint16_t address, const uint8_t value);
	uint8_t read8(const uint16_t address) const;

	void doEventSoundLength();

	inline float getSample(float amplitude) {
		if(!isOn()) { return 0; }

		float volume;
		switch((m_nr32 >> 5) & 0x03) {
		case 0: volume = 0.0f; break;
		case 1: volume = 1.0f; break;
		case 2: volume = 0.5f; break;
		case 3: volume = 0.25f; break;
		}

		uint8_t value = m_wave[m_waveIndex / 2];
		value = ((m_waveIndex % 2 == 0) ? (value >> 4) : value) & 0xF;

		float normalizedValue = ((static_cast<float>(value) * volume) / 15.0f) * 2.0f - 1.0f;
		return normalizedValue * amplitude;
	}

	inline bool isOn() const { return m_isOn; }
	inline bool isDacOn() const { return (m_nr30 & 0x80) != 0; }

  private:
	inline void turnOff() { m_isOn = false; }
	inline void turnOn() { m_isOn = true; }
	inline double getFrequency() const { return 65536.0 / (2048 - m_periodDivider); }

	bool m_isOn;

	double m_phase;

	uint8_t m_nr30, m_nr31, m_nr32;

	bool m_lengthEnabled;
	uint16_t m_lengthTimer;
	uint16_t m_periodDivider;

	uint8_t m_wave[32];
	uint8_t m_waveIndex;
};
} // namespace Zirc
