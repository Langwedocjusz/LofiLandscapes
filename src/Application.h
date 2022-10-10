#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"

class Application{
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    ~Application();

    void Run();
    void OnEvent(Event& e);
private:
    bool m_ShowMenu = true;

    Window m_Window;
    Renderer m_Renderer;
    Timer m_Timer;
};
