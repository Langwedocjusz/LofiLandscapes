#include "Window.h"

#include "glad/glad.h"

#include <iostream>

Window::Window(const std::string& title, unsigned int width, unsigned int height) {
    m_WindowData.Title  = title;
    m_WindowData.Width  = width;
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

    //user pointer:
    glfwSetWindowUserPointer(m_Window, &m_WindowData);

    //Opengl error callback:
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, 
                              GLenum severity, GLsizei length, const GLchar* message,
                              const void* userParam) 
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return ;

        std::cout << "GL CALLBACK: type = " << type 
                  << ", severity = " << severity 
                  << ", message = " << message << '\n';
    }, 0);

    //Glfw callbacks:
    glfwSetErrorCallback([](int code, const char* message){
        std::cerr << "Glfw Error: \n" << "Code: " << code
                  << "Message: " << message << '\n';
    });

    glfwSetFramebufferSizeCallback(m_Window, 
        [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
        });

    glfwSetKeyCallback(m_Window, 
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch(action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, false);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, true);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
            }
        });

    glfwSetCursorPosCallback(m_Window, 
        [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event(xPos, yPos);
            data.EventCallback(event);
        });

    glfwSetMouseButtonCallback(m_Window, 
        [](GLFWwindow* window, int button, int action, int mods) 
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MousePressedEvent event(button, mods);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent event(button, mods);
                    data.EventCallback(event);
                    break;
                }
            }
        });

    //Enable vsync
    glfwSwapInterval(1);
    
    //Set initial viewport dimensions
    glViewport(0, 0, m_WindowData.Width, m_WindowData.Height);
}

Window::~Window() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::OnUpdate() {
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}

void Window::CaptureCursor() {
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::FreeCursor() {
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void Window::setEventCallback(std::function<void(Event&)> callback) {
    m_WindowData.EventCallback = callback;
}

