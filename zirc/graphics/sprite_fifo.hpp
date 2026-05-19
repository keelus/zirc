#pragma once

#include <cassert>
#include <cstdint>
#include <deque>

#include "../bus.hpp"

namespace Zirc {
extern Color colorPalettes[3][4];

class SpriteFifo {
  public:
	SpriteFifo(Bus &bus) : m_bus(bus) { reset(0); }

	struct SpritePixel {
		Color color;
		bool behindBg;
		bool isTransparent;
	};

	bool tickDot(const uint8_t ly);
	bool pop(SpritePixel &spritePixel);
	void reset(uint8_t fetchingSpriteIndex);

  private:
	inline void getTileHLine(uint8_t &byte0, uint8_t &byte1) const {
		uint16_t tileAddress = 0x8000 | (static_cast<uint16_t>(m_tileIndex) * 16);

		byte0 = m_bus.read8(static_cast<uint16_t>(tileAddress + m_localY * 2));
		byte1 = m_bus.read8(static_cast<uint16_t>(tileAddress + m_localY * 2 + 1));
	}

	enum class State {
		FetchTileNumber = 0,
		FetchLow = 1,
		FetchHigh = 2,
		Push = 3,
	};
	State m_state;
	uint8_t m_dotsCurrentState = 0;

	struct Pixel {
		uint8_t color;
		uint8_t x;
		uint8_t y;
		bool behindBg;
	};
	std::deque<Pixel> m_pixels;

	uint8_t m_tileLow, m_tileHigh;
	uint16_t m_tileIndex;

	uint8_t m_spriteX, m_spriteY;
	uint8_t m_spriteAttrs, m_spriteIndex;

	uint8_t m_localY;
	bool m_drawingBottom = false;

	Bus &m_bus;
};
} // namespace Zirc
