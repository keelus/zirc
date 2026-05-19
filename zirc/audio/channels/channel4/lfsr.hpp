#pragma once

#include <cstdint>

namespace Zirc {
class Lfsr {
  public:
	inline void tick() {
		uint16_t result = (m_currentState & 0x1) == ((m_currentState >> 1) & 0x1);

		m_currentState &= static_cast<uint16_t>(~(1 << 15));
		m_currentState |= static_cast<uint16_t>(result << 15);

		if(m_isShortMode) {
			m_currentState &= static_cast<uint16_t>(~(1 << 7));
			m_currentState |= static_cast<uint16_t>(result << 7);
		}

		m_currentState >>= 1;
	}

	inline void reset() { m_currentState = 0x7FF; }

	inline uint8_t getBit() const { return m_currentState & 0x1; }

	inline void setShortMode() { m_isShortMode = true; }
	inline void setLongMode() { m_isShortMode = false; }

  private:
	bool m_isShortMode;
	uint16_t m_currentState;
};
} // namespace Zirc
