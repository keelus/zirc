#pragma once

#include <cstdint>

#include "audio/apu.hpp"
#include "cartridge/cartridge.hpp"
#include "joypad.hpp"
#include "memory.hpp"
#include "timer.hpp"

namespace Zirc {
#define IO_MEM_SIZE 128

class Cpu;
class Ppu;

class Bus {
  public:
	enum class InterruptRequestType {
		Joypad,
		Serial,
		Timer,
		Lcd,
		VBlank,
	};
	Bus() { reset(); }

	inline void addApu(Apu *apu) { m_apu = apu; }
	inline void addCartridge(Cartridge *cartridge) { m_cartridge = cartridge; }
	inline void addCpu(Cpu *cpu) { m_cpu = cpu; }
	inline void addJoypad(Joypad *joypad) { m_joypad = joypad; }
	inline void addMemory(Memory *memory) { m_memory = memory; }
	inline void addPpu(Ppu *ppu) { m_ppu = ppu; }
	inline void addTimer(Timer *timer) { m_timer = timer; }

	void doDmaTransfer(void);

	uint8_t read8(const uint16_t address) const;
	uint16_t read16(const uint16_t address) const;

	void write8(const uint16_t address, const uint8_t value);
	void write16(const uint16_t address, const uint16_t value);

	void requestInterrupt(InterruptRequestType interruptType);

	inline bool introEnded() const { return m_introEnded; }

	inline void reset() {
		m_introEnded = false;
		m_oamSourceAndStart = 0;
	}

  private:
	uint8_t ioRead8(const uint16_t address) const;
	void ioWrite8(const uint16_t address, const uint8_t value);

	bool m_introEnded;

	Apu *m_apu = nullptr;
	Cartridge *m_cartridge = nullptr;
	Cpu *m_cpu = nullptr;
	Joypad *m_joypad = nullptr;
	Memory *m_memory = nullptr;
	Ppu *m_ppu = nullptr;
	Timer *m_timer = nullptr;

	uint8_t m_oamSourceAndStart;
};
} // namespace Zirc
