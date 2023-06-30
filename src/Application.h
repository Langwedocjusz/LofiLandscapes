#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"

class Application{
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    ~Application();

    void StartMenu();
    void Init();
    void Run();

    void OnEvent(Event& e);
private:
    void StartFrame();
    void EndFrame();

    bool m_ShowMenu = true;

    bool m_ShowStartMenu = true;
    Renderer::StartSettings m_StartSettings;

    Window m_Window;
    Renderer m_Renderer;
    Timer m_Timer;
};
