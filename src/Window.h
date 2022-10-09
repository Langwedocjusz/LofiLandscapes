#pragma once

#include "Events.h"

#include "GLFW/glfw3.h"

#include <string>
#include <functional>

struct WindowData {
    std::string Title;
    unsigned int Width, Height;

    std::function<void(Event&)> EventCallback;
};

class Window{
public:
    Window(const std::string& title, unsigned int width, unsigned int height);
    ~Window();

    void OnUpdate();

    bool ShouldClose();
    GLFWwindow* getGLFWPointer() {return m_Window;}
    void setEventCallback(std::function<void(Event&)> callback);
private:
    GLFWwindow* m_Window;
    WindowData m_WindowData;
};
