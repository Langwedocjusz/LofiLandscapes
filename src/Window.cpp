#include "Window.h"

#include "glad/glad.h"

Window::Window(const std::string& title, unsigned int width, unsigned int height) {
    m_WindowData.Title = title;
    m_WindowData.Width = width;
    m_WindowData.Height = height;

    //initialize GLFW:
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(m_WindowData.Width, m_WindowData.Height,
                                m_WindowData.Title.c_str(), NULL, NULL);

    if (m_Window == nullptr)
        throw "Failed to initialize glfw!";

    glfwMakeContextCurrent(m_Window);

    //initialize GLAD:
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw "Failed to initialize glad!";
}

Window::~Window() {
    glfwTerminate();
}

void Window::OnUpdate() {
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(m_Window);
}
