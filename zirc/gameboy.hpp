#pragma once

#include <cstdint>
#include <memory>

#include "audio/apu.hpp"
#include "bus.hpp"
#include "cartridge/cartridge.hpp"
#include "cpu/cpu.hpp"
#include "graphics/lcd.hpp"
#include "graphics/ppu.hpp"
#include "joypad.hpp"
#include "memory.hpp"
#include "timer.hpp"

namespace Zirc {
class GameBoy {
  public:
	GameBoy(const std::string &cartridgePath, Platform &platform)
		: m_apu(platform), m_cpu(m_bus), m_lcd(platform), m_ppu(m_bus, m_lcd), m_timer(m_bus) {
		m_cartridge = Cartridge::createCartridge(cartridgePath);

		m_bus.addApu(&m_apu);
		m_bus.addCartridge(m_cartridge.get());
		m_bus.addCpu(&m_cpu);
		m_bus.addJoypad(&m_joypad);
		m_bus.addMemory(&m_memory);
		m_bus.addPpu(&m_ppu);
		m_bus.addTimer(&m_timer);
	}

	void start(void);
	int tick();
	void reset();

	inline bool introEnded() const { return m_bus.introEnded(); }

	void loadCustomBootRom(const std::string &bootRomPath);
	void disableCustomBootRom() { m_cartridge->disableCustomBootRom(); }

	inline void handleKeydown(Joypad::Key key) { m_joypad.handleKeyDown(key); }
	inline void handleKeyup(Joypad::Key key) { m_joypad.handleKeyUp(key); }

	inline void debugCartridge(void) const { m_cartridge->debug(); }
	inline void dump(void) { m_cpu.dump(); }

	static constexpr float FRAMES_PER_SECOND = 59.7f;
	static constexpr float CYCLES_PER_FRAME = (Cpu::CLOCK_SPEED / FRAMES_PER_SECOND);
	static constexpr float MS_PER_FRAME = 1 / FRAMES_PER_SECOND * 1000;

  private:
	uint16_t m_prevDiv = 0;

	Apu m_apu;
	Bus m_bus;
	Cpu m_cpu;
	Joypad m_joypad;
	Lcd m_lcd;
	Memory m_memory;
	Ppu m_ppu;
	Timer m_timer;
	std::unique_ptr<Cartridge> m_cartridge;
};
} // namespace Zirc
