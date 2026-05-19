#pragma once

#include <SDL.h>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <zirc/audio/ringbuffer.hpp>
#include <zirc/config.hpp>
#include <zirc/gameboy.hpp>
#include <zirc/graphics/lcd.hpp>
#include <zirc/joypad.hpp>
#include <zirc/platform.hpp>

#define SCALE 5
#define FRAME_BUFFER_SIZE (Zirc::Lcd::WIDTH * Zirc::Lcd::HEIGHT)

class PlatformSdl2 : public Zirc::Platform {
  public:
	PlatformSdl2(bool limitFps);
	~PlatformSdl2() {
		if(m_texture) { SDL_DestroyTexture(m_texture); }
		if(m_renderer) { SDL_DestroyRenderer(m_renderer); }
		if(m_window) { SDL_DestroyWindow(m_window); }
		SDL_Quit();
	}

	bool running() const override { return m_running; }

	void drawPixel(uint8_t x, uint8_t y, Zirc::Color color) override {
		getBackBuffer()[y * Zirc::Lcd::WIDTH + x] = (static_cast<uint32_t>(color.red()) << 16) |
													(static_cast<uint32_t>(color.green()) << 8) |
													(static_cast<uint32_t>(color.blue()));
	}

	void beforeFrame() override {
		m_frameStart = SDL_GetTicks();

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			assert(m_gameBoy && "PlatformSdl2's GameBoy was not initialized.");

			if(event.type == SDL_QUIT) {
				m_running = false;
			} else if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				switch(event.key.keysym.sym) {
				case SDLK_1: m_nextActiveColorPalette = 0; break;
				case SDLK_2: m_nextActiveColorPalette = 1; break;
				case SDLK_3: m_nextActiveColorPalette = 2; break;
				default: break;
				}

				Zirc::Joypad::Key key;
				switch(event.key.keysym.sym) {
				case SDLK_UP: key = Zirc::Joypad::Key::Up; break;
				case SDLK_DOWN: key = Zirc::Joypad::Key::Down; break;
				case SDLK_LEFT: key = Zirc::Joypad::Key::Left; break;
				case SDLK_RIGHT: key = Zirc::Joypad::Key::Right; break;
				case SDLK_RETURN: key = Zirc::Joypad::Key::Start; break;
				case SDLK_BACKSPACE: key = Zirc::Joypad::Key::Select; break;
				case SDLK_a: key = Zirc::Joypad::Key::A; break;
				case SDLK_b: key = Zirc::Joypad::Key::B; break;
				default: continue;
				}

				if(event.type == SDL_KEYDOWN) {
					m_gameBoy->handleKeydown(key);
				} else {
					m_gameBoy->handleKeyup(key);
				}
			}
		}

		SDL_RenderClear(m_renderer);
	}

	void afterFrame() override {
		if(m_limitFps) {
			Uint32 frameMs = SDL_GetTicks() - m_frameStart;
			if(frameMs < Zirc::GameBoy::MS_PER_FRAME) { SDL_Delay(Zirc::GameBoy::MS_PER_FRAME - frameMs); }
		}
	}

	void swapBuffers() override {
		m_backBufferIndex = !m_backBufferIndex;
		std::memset(getBackBuffer(), 0, sizeof(uint32_t) * FRAME_BUFFER_SIZE);
		Zirc::Config::get().activeColorPalette = m_nextActiveColorPalette;
	}

	void showFrame() override {
		SDL_UpdateTexture(m_texture, NULL, getFrontBuffer(), Zirc::Lcd::WIDTH * sizeof(uint32_t));
		SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
		SDL_RenderPresent(m_renderer);
	}

	float getAudioAmplitude() const override { return 1; }
	float getAudioSampleRate() const override { return AUDIO_SAMPLE_RATE; }

	void pushAudioSample(float sample) override { m_audioSampleBuffer.pushSample(sample); }

	void muteAudio() override { SDL_PauseAudio(true); }
	void unmuteAudio() override { SDL_PauseAudio(false); }

  private:
	static void audioCallback(void *userData, Uint8 *stream, int len) {
		PlatformSdl2 *platform = static_cast<PlatformSdl2 *>(userData);

		int samples = len / sizeof(float);
		float *buffer = (float *)stream;

		std::memset(buffer, 0, len);

		for(size_t i = 0; i < samples; i++) {
			if(!platform->m_audioSampleBuffer.popSample(buffer[i])) { buffer[i] = 0.0f; }
		}
	}

	const uint32_t *getFrontBuffer() const { return m_buffers[m_backBufferIndex ? 0 : 1]; }
	uint32_t *getFrontBuffer() { return m_buffers[m_backBufferIndex ? 0 : 1]; }
	uint32_t *getBackBuffer() { return m_buffers[m_backBufferIndex ? 1 : 0]; }

	uint8_t m_nextActiveColorPalette = Zirc::Config::get().activeColorPalette;

	bool m_running = true;

	Uint32 m_frameStart = 0;
	bool m_limitFps = false;

	uint32_t m_buffers[2][FRAME_BUFFER_SIZE] = {0};
	uint32_t m_backBufferIndex = 0;

	SDL_Window *m_window = nullptr;
	SDL_Renderer *m_renderer = nullptr;
	SDL_Texture *m_texture = nullptr;
	SDL_AudioSpec m_spec;

	Zirc::AudioRingBuffer<4096> m_audioSampleBuffer;

	static constexpr float AUDIO_SAMPLE_RATE = 44100.0;
	static constexpr size_t AUDIO_SAMPLE_AMOUNT = 1024;
};
