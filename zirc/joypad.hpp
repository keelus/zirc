#pragma once

#include <bitset>
#include <cstdint>

namespace Zirc {
class Joypad {
  public:
	Joypad() { reset(); }

	inline void write8(const uint8_t value) {
		uint8_t outputBits = value & 0x30;
		if(outputBits == 0x30) {
			m_selectedOutput = SelectedOutput::None;
		} else if(outputBits == 0x20) {
			m_selectedOutput = SelectedOutput::DPad;
		} else {
			m_selectedOutput = SelectedOutput::Buttons;
		}
	}

	inline uint8_t read8() const {
		switch(m_selectedOutput) {
		case SelectedOutput::Buttons: return 0x30 | (static_cast<uint8_t>(m_buttons.to_ulong()) & 0xF); break;
		case SelectedOutput::DPad: return 0x30 | (static_cast<uint8_t>(m_dPad.to_ulong()) & 0xF); break;
		default: return 0x3F;
		}
	}

	enum class Key { A, B, Select, Start, Right, Left, Up, Down };

	inline void handleKeyDown(Key key) { handleKey(key, false); }
	inline void handleKeyUp(Key key) { handleKey(key, true); }

	inline void reset() {
		m_buttons = 0xF;
		m_dPad = 0xF;

		m_selectedOutput = SelectedOutput::None;
	}

  private:
	enum SelectedOutput { None = 0, Buttons, DPad };

	inline void handleKey(Key key, bool isKeyUp) {
		uint8_t keyValue = static_cast<uint8_t>(key);
		if(keyValue & 0b100) {
			m_dPad.set(keyValue & 0b11, isKeyUp);
		} else {
			m_buttons.set(keyValue & 0b11, isKeyUp);
		}
	}

	std::bitset<4> m_buttons, m_dPad;
	SelectedOutput m_selectedOutput;
};
} // namespace Zirc
