#pragma once

#include "Events.h"

#include "GLFW/glfw3.h"

#include <string>
#include <functional>
#include <cstdint>

struct WindowData {
    std::string Title;
    uint32_t Width, Height;

    std::function<void(Event&)> EventCallback;
};

class Window{
public:
    Window(const std::string& title, uint32_t width, uint32_t height);
    ~Window();

    void OnUpdate();

    void CaptureCursor();
    void FreeCursor();

    bool ShouldClose();
    GLFWwindow* getGLFWPointer() {return m_Window;}

    uint32_t getWidth()  const {return m_WindowData.Width;}
    uint32_t getHeight() const {return m_WindowData.Height;}

    void setEventCallback(std::function<void(Event&)> callback);
private:
    GLFWwindow* m_Window;
    WindowData m_WindowData;
};
