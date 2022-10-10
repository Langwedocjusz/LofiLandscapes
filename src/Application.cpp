#include "Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Keycodes.h"

Application::Application(const std::string& title, unsigned int width, unsigned int height) 
    : m_Window(title, width, height), m_Renderer(width, height) 
{
    //initialize ImGui:
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGui_ImplGlfw_InitForOpenGL(m_Window.getGLFWPointer(), true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    //Redirect window callbacks to application's on event function
    m_Window.setEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
}

Application::~Application() {}

void Application::Run() {
    while (!m_Window.ShouldClose()) {
        m_Timer.Update();
        m_Renderer.OnUpdate(m_Timer.getDeltaTime());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (m_ShowMenu) m_Renderer.OnImGuiRender();
        m_Renderer.OnRender();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_Window.OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
   EventType type = e.getEventType();

   switch(type) {
        case EventType::WindowResize: {
            WindowResizeEvent* ptr = dynamic_cast<WindowResizeEvent*>(&e);
            m_Renderer.OnWindowResize(ptr->getWidth(), ptr->getHeight());
            break; 
        }
        case EventType::KeyPressed: {
            KeyPressedEvent* ptr = dynamic_cast<KeyPressedEvent*>(&e);

            if(ptr->getKeycode() == LOFI_KEY_ESCAPE) {
                if (m_ShowMenu) {
                    m_Renderer.RestartMouse();
                    m_Window.CaptureCursor();
                }
                else
                    m_Window.FreeCursor();

                m_ShowMenu = (!m_ShowMenu);
            }

            else if(!m_ShowMenu)
                m_Renderer.OnKeyPressed(ptr->getKeycode(), ptr->getRepeat());

            break;
        }
        case EventType::KeyReleased: {
            if (!m_ShowMenu) {
                KeyReleasedEvent* ptr = dynamic_cast<KeyReleasedEvent*>(&e);
                m_Renderer.OnKeyReleased(ptr->getKeycode());
            }
            break;
        }
        case EventType::MouseMoved: {
            if (!m_ShowMenu) {
                MouseMovedEvent* ptr = dynamic_cast<MouseMovedEvent*>(&e);
                m_Renderer.OnMouseMoved(ptr->getX(), ptr->getY());
            }
            break;
        }
   }
}

