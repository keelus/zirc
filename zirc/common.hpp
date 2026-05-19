#pragma once

#include <cstdint>

#define IN_RANGE(address, start, end) ((address) >= (start) && (address) <= (end))

namespace Zirc {
class Color {
  public:
	Color() {}
	Color(uint8_t red, uint8_t green, uint8_t blue) {
		m_red = red;
		m_green = green;
		m_blue = blue;
	}

	inline uint8_t red() const { return m_red; }
	inline uint8_t green() const { return m_green; }
	inline uint8_t blue() const { return m_blue; }

  private:
	uint8_t m_red = 0;
	uint8_t m_green = 0;
	uint8_t m_blue = 0;
};
} // namespace Zirc
