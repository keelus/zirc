#pragma once

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "../common.hpp"
#include "../platform.hpp"
#include "channels/channel1/channel1.hpp"
#include "channels/channel2/channel2.hpp"
#include "channels/channel3/channel3.hpp"
#include "channels/channel4/channel4.hpp"

namespace Zirc {
#define APU_RATE (1 << 20)

class Apu {
  public:
	Apu(Platform &platform) : m_platform(platform) { reset(); }

	void write8(const uint16_t address, const uint8_t value) {
		if(IN_RANGE(address, 0xFF10, 0xFF14)) { return m_channel1.write8(address, value); }
		if(IN_RANGE(address, 0xFF16, 0xFF19)) { return m_channel2.write8(address, value); }
		if(IN_RANGE(address, 0xFF1A, 0xFF1E) || IN_RANGE(address, 0xFF30, 0xFF3F)) {
			return m_channel3.write8(address, value);
		}
		if(IN_RANGE(address, 0xFF20, 0xFF23)) { return m_channel4.write8(address, value); }

		switch(address) {
		case 0xFF26: m_audioEnabled = (value & 0x80) != 0; break;
		default: {
			std::stringstream stream;
			stream << "Audio: Illegal write on address 0x" << std::hex << std::setw(4) << std::setfill('0')
				   << uint(address) << std::endl;
			throw std::runtime_error(stream.str());
		}
		}
	}

	uint8_t read8(const uint16_t address) const {
		if(IN_RANGE(address, 0xFF10, 0xFF14)) { return m_channel1.read8(address); }
		if(IN_RANGE(address, 0xFF16, 0xFF19)) { return m_channel2.read8(address); }
		if(IN_RANGE(address, 0xFF1A, 0xFF1E) || IN_RANGE(address, 0xFF30, 0xFF3F)) { return m_channel3.read8(address); }
		if(IN_RANGE(address, 0xFF20, 0xFF23)) { return m_channel4.read8(address); }

		switch(address) {
		case 0xFF26:
			return (m_audioEnabled ? (1 << 7) : 0) | (m_channel4.isOn() ? (1 << 3) : 0) |
				   (m_channel3.isOn() ? (1 << 2) : 0) | (m_channel2.isOn() ? (1 << 1) : 0) |
				   (m_channel1.isOn() ? 1 : 0);
		default: {
			std::stringstream stream;
			stream << "Audio: Illegal read on address 0x" << std::hex << std::setw(4) << std::setfill('0')
				   << uint(address) << std::endl;
			throw std::runtime_error(stream.str());
		}
		}
	}


	void increaseDiv() {
		m_divApu++;

		if(m_divApu % 8 == 0) {
			m_channel1.doEventEnvelopeSweep();
			m_channel2.doEventEnvelopeSweep();
			m_channel4.doEventEnvelopeSweep();
		}
		if(m_divApu % 2 == 0) {
			m_channel1.doEventSoundLength();
			m_channel2.doEventSoundLength();
			m_channel3.doEventSoundLength();
			m_channel4.doEventSoundLength();
		}
		if(m_divApu % 4 == 0) { m_channel1.doEventFrequencySweep(); }
	}

	void tick(const uint8_t tStates) {
		assert(tStates % 4 == 0);

		for(uint8_t i = 0; i < tStates; i += 4) {
			m_channel1.tick(APU_RATE);
			m_channel2.tick(APU_RATE);
			m_channel3.tick(APU_RATE);
			m_channel4.tick(APU_RATE);

			float sample1 = m_channel1.getSample(m_platform.getAudioAmplitude());
			float sample2 = m_channel2.getSample(m_platform.getAudioAmplitude());
			float sample3 = m_channel3.getSample(m_platform.getAudioAmplitude());
			float sample4 = m_channel4.getSample(m_platform.getAudioAmplitude());

			float sample = (sample1 + sample2 + sample3 + sample4) / 4.0f;

			pushSample(sample);
		}

		bool allChannelsOff = !(m_channel1.isOn() || m_channel2.isOn() || m_channel3.isOn() || m_channel4.isOn());
		if(allChannelsOff != m_prevAllChannelsOff) {
			if(allChannelsOff) {
				m_platform.muteAudio();
			} else {
				m_platform.unmuteAudio();
			}
		}
		m_prevAllChannelsOff = allChannelsOff;
	}

	void reset() {
		m_prevAllChannelsOff = false;
		m_audioEnabled = false;
		m_divApu = 0;

		m_sampleAccumulator = 0.0;

		m_channel1.reset();
		m_channel2.reset();
		m_channel3.reset();
		m_channel4.reset();
	}

  private:
	void pushSample(float sample) {
		m_sampleAccumulator += 1.0;
		if(m_sampleAccumulator >= getSampleRatio()) {
			m_sampleAccumulator -= getSampleRatio();
			m_platform.pushAudioSample(sample);
		}
	}

	float getSampleRatio() const { return APU_RATE / m_platform.getAudioSampleRate(); }

	bool m_prevAllChannelsOff;
	bool m_audioEnabled;
	uint8_t m_divApu;

	Channel1 m_channel1;
	Channel2 m_channel2;
	Channel3 m_channel3;
	Channel4 m_channel4;

	double m_sampleAccumulator;

	Platform &m_platform;
};
} // namespace Zirc
