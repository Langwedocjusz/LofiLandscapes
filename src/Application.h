#pragma once

#include "Window.h"
#include "Renderer.h"

class Application{
public:
    Application(const std::string& title, unsigned int width, unsigned int height);
    ~Application();

    void Run();
private:
    Window m_Window;
    Renderer m_Renderer;
};
