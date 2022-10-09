#include "Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

Application::Application(const std::string& title, unsigned int width, unsigned int height) : m_Window(title, width, height) 
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
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        m_Renderer.OnImGuiUpdate();
        m_Renderer.OnUpdate();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_Window.OnUpdate();
    }
}

void Application::OnEvent(Event& e) {
   EventType type = e.getEventType();

   switch(type) {
        case EventType::KeyPressed:
            KeyPressedEvent* ptr = dynamic_cast<KeyPressedEvent*>(&e);
            m_Renderer.OnKeyPressed(ptr->getKeycode(), ptr->getRepeat());
            break;
   }
}

