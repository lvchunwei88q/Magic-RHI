#pragma once
#include <chrono>
#include <functional>

class Debouncer {
public:
    explicit Debouncer(int stableMs, std::function<void()> callback)
        : m_stableDuration(stableMs), m_callback(callback) {
    }

    void update() {
        m_lastUpdateTime = std::chrono::steady_clock::now();
        m_hasPending = true;
    }

    void tick() {
        if (!m_hasPending) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdateTime);

        if (elapsed.count() >= m_stableDuration) {
            if (m_callback) m_callback();
            m_hasPending = false;
        }
    }

private:
    int m_stableDuration;
    std::function<void()> m_callback;
    std::chrono::steady_clock::time_point m_lastUpdateTime;
    bool m_hasPending = false;
};