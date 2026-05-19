#include "../config.hpp"
#include "background_fifo.hpp"

namespace Zirc {
extern Color colorPalettes[3][4];

void BackgroundFifo::reset(const uint8_t scx, bool isWindow) {
	m_state = State::FetchTileNumber;
	m_dotsCurrentState = 0;

	m_pixels.clear();

	m_xFetch = 0;

	m_firstCopyPixelsRemaining = 8;

	m_pixelsOddDiscardRemaining = scx % 8;

	m_isWindow = isWindow;
}

void BackgroundFifo::tickDot(const uint8_t ly, const uint8_t scy, const uint8_t scx, const uint8_t wly) {
	if(m_lcd.screenX() >= Lcd::WIDTH) { return; }

	m_dotsCurrentState++;

	switch(m_state) {
	case State::FetchTileNumber: {
		if(m_dotsCurrentState == 2 && !m_isWindow) {
			m_tileOffset = ((scx) / 8 + m_xFetch) & 0x1F;
			m_tileOffset += static_cast<uint16_t>(32 * (((ly + scy) & 0xFF) / 8));
			m_tileOffset &= 0x3FF;

			m_state = State::FetchLow;
			m_dotsCurrentState = 0;
		} else if(m_dotsCurrentState == 2 && m_isWindow) {
			m_tileOffset = m_xFetch;
			m_tileOffset += static_cast<uint16_t>(32 * (wly / 8));
			m_tileOffset &= 0x3FF;

			m_state = State::FetchLow;
			m_dotsCurrentState = 0;
		}
		break;
	}
	case State::FetchLow: {
		if(m_dotsCurrentState == 2) {
			uint8_t byte1;
			uint8_t offset = 2 * ((ly + scy) % 8);
			if(m_isWindow) { offset = 2 * (wly % 8); }
			getTileHLine(m_tileOffset, offset, m_tileLow, byte1,
						 ((m_bus.read8(0xFF40) >> (m_isWindow ? 6 : 3)) & 1) != 0);

			m_state = State::FetchHigh;
			m_dotsCurrentState = 0;
		}
		break;
	}
	case State::FetchHigh: {
		if(m_dotsCurrentState == 2) {
			uint8_t byte0;
			uint8_t offset = 2 * ((ly + scy) % 8);
			if(m_isWindow) { offset = 2 * (wly % 8); }
			getTileHLine(m_tileOffset, offset, byte0, m_tileHigh,
						 ((m_bus.read8(0xFF40) >> (m_isWindow ? 6 : 3)) & 1) != 0);

			m_state = State::Push;
			m_dotsCurrentState = 0;
		}
		break;
	}
	case State::Push: {
		if(m_dotsCurrentState == 2) {
			if(m_pixels.empty()) {
				for(int i = 0; i < 8; i++) {
					uint8_t lower = (m_tileLow >> (7 - i)) & 1;
					uint8_t upper = (m_tileHigh >> (7 - i)) & 1;
					uint8_t colorId = static_cast<uint8_t>((upper << 1) | lower);

					bool bgEnabled = (m_bus.read8(0xFF40)) & 0x1;
					if(bgEnabled) {
						m_pixels.push_back({colorId});
					} else {
						m_pixels.push_back({0});
					}
				}

				m_xFetch++;
				m_state = State::FetchTileNumber;
				m_dotsCurrentState = 0;
			} else {
				m_dotsCurrentState--;
			}
		}
		break;
	}
	}
}

bool BackgroundFifo::pop(BgPixel &bgPixel) {
	if(m_pixels.empty() || m_lcd.screenX() >= Lcd::WIDTH) { return false; }

	Pixel px = m_pixels.front();
	m_pixels.pop_front();

	uint8_t palette = m_bus.read8(0xFF47);
	uint8_t shade = (palette >> (px.color * 2)) & 0b11;

	if(m_firstCopyPixelsRemaining == 0) {
		if(m_pixelsOddDiscardRemaining == 0) {
			bgPixel.color = colorPalettes[Config::get().activeColorPalette][shade];
			bgPixel.isTransparent = px.color == 0;

			return true;
		} else {
			m_pixelsOddDiscardRemaining--;
		}
	} else {
		m_firstCopyPixelsRemaining--;
		m_xFetch = 0;
	}

	return false;
}
} // namespace Zirc
