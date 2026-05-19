#pragma once

#include <atomic>
#include <cstddef>
#include <cstring>

namespace Zirc {
template <std::size_t Size> class AudioRingBuffer {
  public:
	AudioRingBuffer() { reset(); }

	bool pushSample(const float &sample) {
		size_t currentHead = m_head.load(std::memory_order_relaxed);
		size_t nextHead = (currentHead + 1) % Size;
		if(nextHead == m_tail.load(std::memory_order_acquire)) { return false; }

		m_buffer[currentHead] = sample;
		m_head.store(nextHead, std::memory_order_release);

		return true;
	}

	bool popSample(float &sample) {
		size_t currentTail = m_tail.load(std::memory_order_relaxed);
		if(currentTail == m_head.load(std::memory_order_acquire)) { return false; }

		sample = m_buffer[currentTail];
		m_tail.store((currentTail + 1) % Size, std::memory_order_release);

		return true;
	}

	void reset() {
		std::memset(m_buffer, 0, sizeof(m_buffer));
		m_head = 0;
		m_tail = 0;
	}

  private:
	float m_buffer[Size];
	std::atomic<size_t> m_head{0};
	std::atomic<size_t> m_tail{0};
};
} // namespace Zirc
