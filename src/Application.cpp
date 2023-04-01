#include "Application.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ImGuiStyles.h"
#include "ImGuiUtils.h"

#include "Keycodes.h"

#include "glad/glad.h"

Application::Application(const std::string& title, unsigned int width, unsigned int height) 
    : m_Window(title, width, height), m_Renderer(width, height) 
{
    //Initialize ImGui:
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
   
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(m_Window.getGLFWPointer(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGui::StyleColorsDark();
   
    //Set ImGui Style::
    ImGuiStyles::OverShiftedDarkMode();

    //Set ImGui Font:
    io.Fonts->AddFontFromFileTTF("res/fonts/OpenSans/OpenSans-Medium.ttf", 16);

    //Redirect window callbacks to application's on event function
    m_Window.setEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
}

Application::~Application() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::StartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::EndFrame() {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(m_Window.getWidth(), m_Window.getHeight());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_Window.OnUpdate();
}

void Application::StartMenu() {
    while (m_ShowStartMenu) {
        StartFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("Start settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        
        ImGuiUtils::SliderInt("Grid subdivisions", &m_StartSettings.Subdivisions, 16, 96);
        ImGuiUtils::SliderInt("Lod levels", &m_StartSettings.LodLevels, 1, 10);
        ImGuiUtils::SliderInt("Heightmap resolution", &m_StartSettings.HeightRes, 256, 4096);
        ImGuiUtils::SliderInt("Shadowmap resolution", &m_StartSettings.ShadowRes, 256, 4096);

        //-----World type selection----------------------
        //To do: Imgui combo abstraction
        const char* items[] = {"finite", "tiling"};
        static const char* current_item = items[0];

        ImGuiStyle& style = ImGui::GetStyle();
        float padding = style.FramePadding.x;

        ImGui::Text("World type");
        ImGui::SameLine(ImGui::GetWindowWidth() / 3);
        ImGui::PushItemWidth(-padding);

        if (ImGui::BeginCombo("##combo", current_item))
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                bool is_selected = (current_item == items[n]);

                if (ImGui::Selectable(items[n], is_selected)) {
                    current_item = items[n];

                    if (n == 0)
                        m_StartSettings.WrapType = GL_CLAMP_TO_BORDER;
                    else if (n == 1)
                        m_StartSettings.WrapType = GL_REPEAT;
                }     
                
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        //-------------------------------------------------

        if (ImGuiUtils::Button("Start"))
            m_ShowStartMenu = false;

        ImGui::End();

        EndFrame();
    }
}

void Application::InitRenderer() {
    m_Renderer.Init(m_StartSettings);
}

void Application::Run() {
    while (!m_Window.ShouldClose()) {
        m_Timer.Update();
        m_Renderer.OnUpdate(m_Timer.getDeltaTime());

        m_Renderer.OnRender();

        StartFrame();

        if (m_ShowMenu) m_Renderer.OnImGuiRender();

        EndFrame();
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

