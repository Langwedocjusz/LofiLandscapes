#include "Timer.h"

Timer::Timer() {
    m_Start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {}

float Timer::getTime() {
    auto current = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>
        (current - m_Start).count() * 1e-9f;
}

void Timer::Update() {
    float current_time = getTime();
    m_DeltaTime = current_time - m_LastFrame;
    m_LastFrame = current_time;
}
