#pragma once

#include <cstdint>
#include <cstring>
#include <iomanip>

#include "../bus.hpp"
#include "background_fifo.hpp"
#include "lcd.hpp"
#include "sprite_fifo.hpp"

namespace Zirc {
#define PPU_VRAM_SIZE 8192
#define PPU_OAM_SIZE 160

class Ppu {
  public:
	Ppu(Bus &bus, Lcd &lcd) : m_backgroundFifo(bus, lcd), m_spriteFifo(bus), m_bus(bus), m_lcd(lcd) { reset(); }

	void tick(uint8_t cycles);
	void write8(const uint16_t address, const uint8_t value);
	uint8_t read8(const uint16_t address) const;

	inline uint8_t getControl() const { return m_control; }
	inline void setControl(const uint8_t newControl) { m_control = newControl; }

	inline uint8_t getPalette() const { return m_palette; }
	inline void setPalette(const uint8_t newPalette) { m_palette = newPalette; }

	inline uint8_t getObjPalette0() const { return m_objPalette0; }
	inline void setObjPalette0(const uint8_t newPalette) { m_objPalette0 = newPalette; }

	inline uint8_t getObjPalette1() const { return m_objPalette1; }
	inline void setObjPalette1(const uint8_t newPalette) { m_objPalette1 = newPalette; }

	inline uint8_t getScx() const { return m_scx; }
	inline void setScx(const uint8_t newScx) { m_scx = newScx; }

	inline uint8_t getScy() const { return m_scy; }
	inline void setScy(const uint8_t newScy) { m_scy = newScy; }

	inline uint8_t getWx() const { return m_wx; }
	inline void setWx(const uint8_t newWx) { m_wx = newWx; }

	inline uint8_t getWy() const { return m_wy; }
	inline void setWy(const uint8_t newWy) { m_wy = newWy; }

	inline uint8_t getLy() const { return m_ly; }

	inline void setLyc(const uint8_t newLyc) { m_lyc = newLyc; }
	inline uint8_t getLyc() const { return m_ly; }

	inline uint8_t getLcdStatus() const {
		return static_cast<uint8_t>(m_lcdStatus | ((m_lyc == m_ly) << 2) |
									((m_control & 0b10000000) ? 0 : (static_cast<uint8_t>(m_mode) & 0x3)));
	}
	inline void setLcdStatus(const uint8_t newLcdStatus) {
		if(newLcdStatus != 0) {
			std::cout << "LCD_STATUS = 0x" << std::hex << std::setw(2) << std::setfill('0') << uint(newLcdStatus)
					  << std::endl;
		}
		m_lcdStatus = (newLcdStatus & 0xF8) | (m_lcdStatus & 0x7);
	}

	inline void reset() {
		m_yCondition = false;
		m_lycStatRequested = false, m_m0StatRequested = false;
		m_m1StatRequested = false, m_m2StatRequested = false;

		m_cycles = 0;

		std::memset(m_vram, 0, sizeof(m_vram));
		std::memset(m_oam, 0, sizeof(m_oam));

		m_palette = 0;

		m_objPalette0 = 0, m_objPalette1 = 0;

		m_scx = 0, m_scy = 0;
		m_wx = 0, m_wy = 0, m_wly = 0;

		m_ly = 0, m_lyc = 0;

		m_mode = PpuMode::OAM_SCAN;
		m_prevMode = PpuMode::OAM_SCAN;

		m_spritesToDraw.clear();

		m_fetchingSprites = false;
		m_fetchingSpriteIndex = 0;

		m_drawingWindow = false;
		m_control = 0;
		m_lcdStatus = 0;

		m_backgroundFifo.reset(0, false);
		m_spriteFifo.reset(0);
	}

  private:
	void tickDot();
	bool checkSpritesToDraw();

	bool m_yCondition;
	bool m_lycStatRequested, m_m0StatRequested;
	bool m_m1StatRequested, m_m2StatRequested;

	struct Sprite {
		uint8_t index;
		uint8_t x;
	};

	uint16_t m_cycles;

	uint8_t m_vram[PPU_VRAM_SIZE];
	uint8_t m_oam[PPU_OAM_SIZE];

	uint8_t m_palette;
	uint8_t m_objPalette0, m_objPalette1;

	uint8_t m_scx, m_scy;
	uint8_t m_wx, m_wy, m_wly;

	uint8_t m_ly, m_lyc;

	enum class PpuMode { OAM_SCAN = 2, DRAWING = 3, HBLANK = 0, VBLANK = 1 };
	PpuMode m_mode, m_prevMode;

	std::vector<Sprite> m_spritesToDraw;

	bool m_fetchingSprites;
	uint8_t m_fetchingSpriteIndex;

	bool m_drawingWindow;
	uint8_t m_control;
	uint8_t m_lcdStatus;

	BackgroundFifo m_backgroundFifo;
	SpriteFifo m_spriteFifo;

	Bus &m_bus;
	Lcd &m_lcd;
};
} // namespace Zirc
