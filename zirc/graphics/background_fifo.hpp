#pragma once

#include <cstdint>
#include <deque>

#include "../bus.hpp"
#include "lcd.hpp"

namespace Zirc {
class BackgroundFifo {
  public:
	BackgroundFifo(Bus &bus, Lcd &lcd) : m_bus(bus), m_lcd(lcd) { reset(0, false); }


	struct BgPixel {
		Color color;
		bool isTransparent;
	};

	void tickDot(const uint8_t ly, const uint8_t scy, const uint8_t scx, const uint8_t wly);
	bool pop(BgPixel &bgPixel);
	void reset(const uint8_t scx, bool isWindow);

  private:
	inline void getTileHLine(uint16_t tileMapIndex, uint8_t offset, uint8_t &byte0, uint8_t &byte1,
							 uint8_t tileAddressBit) const {
		uint16_t tileAddress = tileAddressBit ? 0x9C00 : 0x9800;
		uint8_t index = m_bus.read8(tileAddress + tileMapIndex);

		uint16_t finalTileAddress;

		uint8_t tileBit = ((m_bus.read8(0xFF40) >> 4) & 1) != 0;
		if(tileBit) {
			finalTileAddress = static_cast<uint16_t>(0x8000 + index * 16);
		} else {
			finalTileAddress = static_cast<uint16_t>(0x9000 + (int8_t)index * 16);
		}

		byte0 = m_bus.read8(finalTileAddress + offset);
		byte1 = m_bus.read8(static_cast<uint16_t>(finalTileAddress + offset + 1));
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
	};
	std::deque<Pixel> m_pixels;

	uint8_t m_tileLow, m_tileHigh;
	uint16_t m_tileOffset = 0;

	uint8_t m_firstCopyPixelsRemaining = 8;
	uint8_t m_pixelsOddDiscardRemaining = 0;

	uint8_t m_xFetch;

	bool m_isWindow = false;

	Bus &m_bus;
	Lcd &m_lcd;
};
} // namespace Zirc
