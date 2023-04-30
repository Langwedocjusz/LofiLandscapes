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

    void CaptureCursor();
    void FreeCursor();

    bool ShouldClose();
    GLFWwindow* getGLFWPointer() {return m_Window;}

    unsigned int getWidth()  const {return m_WindowData.Width;}
    unsigned int getHeight() const {return m_WindowData.Height;}

    void setEventCallback(std::function<void(Event&)> callback);
private:
    GLFWwindow* m_Window;
    WindowData m_WindowData;
};
