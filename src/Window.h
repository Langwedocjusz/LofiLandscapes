#pragma once

#include "GLFW/glfw3.h"

#include <string>

struct WindowData {
    std::string Title;
    unsigned int Width, Height;
};

class Window{
public:
    Window(const std::string& title, unsigned int width, unsigned int height);
    ~Window();

    void OnUpdate();

    bool ShouldClose();
    GLFWwindow* getGLFWPointer() {return m_Window;}
private:
    GLFWwindow* m_Window;
    WindowData m_WindowData;
};
