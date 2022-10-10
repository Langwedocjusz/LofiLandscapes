#pragma once

#include <chrono>

class Timer{
public:
    Timer();
    ~Timer();

    float getTime();
    float getDeltaTime() {return m_DeltaTime;}
    void Update();
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    float m_DeltaTime = 0.0f, m_LastFrame = 0.0f;
};
