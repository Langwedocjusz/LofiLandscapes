#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Timer.h"

#include "glad/glad.h"

struct StartSettings {
    int Subdivisions = 64;
    int LodLevels = 5;
    int HeightRes = 4096;
    int ShadowRes = 4096;
    int WrapType = GL_CLAMP_TO_BORDER;
};

class Application{
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    ~Application();

    void StartMenu();
    void InitRenderer();
    void Run();

    void OnEvent(Event& e);
private:
    bool m_ShowMenu = true;

    bool m_ShowStartMenu = true;
    StartSettings m_StartSettings;

    Window m_Window;
    Renderer m_Renderer;
    Timer m_Timer;
};
