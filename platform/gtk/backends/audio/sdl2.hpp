#pragma once

#include <iostream>
#include <SDL.h>
#include <zirc/audio/ringbuffer.hpp>

#include "backends/audio/audio_backend.hpp"
#include "utils.hpp"

class AudioBackendSdl2 : public AudioBackend {
  public:
	void initialize() override {
		if(SDL_Init(SDL_INIT_AUDIO) != 0) {
			m_initializationError = SDL_GetError();

			std::string errorMsg = "SDL_Init() error: " + m_initializationError;
			std::cerr << "Platform[GTK]: SDL_Init error: " << errorMsg << std::endl;
			showErrorDialog("SDL Audio Error", "Audio could not be initialized: \"" + errorMsg + "\"");

			return;
		}

		m_spec = {0};
		m_spec.freq = AUDIO_SAMPLE_RATE;
		m_spec.format = AUDIO_F32SYS;
		m_spec.channels = 1;
		m_spec.samples = AUDIO_SAMPLE_AMOUNT;
		m_spec.callback = audioCallback;
		m_spec.userdata = this;

		if(SDL_OpenAudio(&m_spec, NULL) < 0) {
			m_initializationError = SDL_GetError();
			SDL_Quit();

			std::string errorMsg = "SDL_OpenAudio() error: " + m_initializationError;
			std::cerr << "Platform[GTK]: SDL_OpenAudio error: " << errorMsg << std::endl;
			showErrorDialog("SDL Audio Error", "Audio could not be initialized: \"" + errorMsg + "\"");

			return;
		}

		m_initialized = true;
	}

	~AudioBackendSdl2() override {
		if(!m_initialized) { return; }

		SDL_CloseAudio();
		SDL_Quit();
	}

	static void audioCallback(void *userData, Uint8 *stream, int len) {
		AudioBackendSdl2 *backend = static_cast<AudioBackendSdl2 *>(userData);

		int samples = len / sizeof(float);
		float *buffer = (float *)stream;

		std::memset(buffer, 0, len);

		for(size_t i = 0; i < samples; i++) {
			if(!backend->m_audioSampleBuffer.popSample(buffer[i])) { buffer[i] = 0.0f; }
		}
	}

	void pushSample(float sample) override { m_audioSampleBuffer.pushSample(sample); }

	void restart() override {
		SDL_PauseAudio(true);
		m_audioSampleBuffer.reset();
	}

	bool isPaused() override { return m_audioPaused; }
	void pause() override {
		m_audioPaused = true;
		SDL_PauseAudio(true);
	}
	void unPause() override {
		m_audioPaused = false;
		SDL_PauseAudio(false);
	}

	bool initialized() const override { return m_initialized; }

  private:
	bool m_initialized = false;
	std::string m_initializationError;

	bool m_audioPaused = false;
	SDL_AudioSpec m_spec;

	Zirc::AudioRingBuffer<4096> m_audioSampleBuffer;

	static constexpr float AUDIO_SAMPLE_RATE = 44100.0;
	static constexpr size_t AUDIO_SAMPLE_AMOUNT = 1024;
};
