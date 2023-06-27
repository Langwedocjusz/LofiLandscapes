#include "Application.h"

#include "imgui.h"
#include "imgui_internal.h"
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

    //Setup handler for additional data in imgui.ini
    m_Renderer.InitImGuiIniHandler();

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
    while (!m_Window.ShouldClose() && m_ShowStartMenu) {
        StartFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("Start settings", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        
        ImGui::BeginChild("#Settings", ImVec2(0.0f, 0.9f*ImGui::GetContentRegionAvail().y), true);
        ImGui::Columns(2, "###col");

        ImGuiUtils::SliderInt("Grid subdivisions", &m_StartSettings.Subdivisions, 16, 96);
        ImGuiUtils::SliderInt("Lod levels", &m_StartSettings.LodLevels, 1, 10);

        ImGui::Spacing(); ImGui::NextColumn(); ImGui::Spacing(); ImGui::NextColumn();

        ImGuiUtils::SliderIntLog("Heightmap resolution", &m_StartSettings.HeightRes, 256, 4096);
        ImGuiUtils::SliderIntLog("Shadowmap resolution", &m_StartSettings.ShadowRes, 256, 4096);
        ImGuiUtils::SliderIntLog("Material resolution", &m_StartSettings.MaterialRes, 256, 4096);

        ImGui::Spacing(); ImGui::NextColumn(); ImGui::Spacing(); ImGui::NextColumn();

        //To do: create custom slider that takes in v, but displayes 2^v
        //For now just round to closest power of 2 manually
        m_StartSettings.HeightRes = std::exp2(std::round(std::log2(m_StartSettings.HeightRes)));
        m_StartSettings.ShadowRes = std::exp2(std::round(std::log2(m_StartSettings.ShadowRes)));
        m_StartSettings.MaterialRes = std::exp2(std::round(std::log2(m_StartSettings.MaterialRes)));

        //-----World type selection----------------------
        std::vector<std::string> options{"finite", "tiling"};

        static int selected_id = 1;

        ImGui::Columns(2, "###col");
        ImGuiUtils::Combo("World type", options, selected_id);

        if (selected_id == 0)
            m_StartSettings.WrapType = GL_CLAMP_TO_BORDER;
        else if (selected_id == 1)
            m_StartSettings.WrapType = GL_REPEAT;

        ImGui::Columns(1, "###col");
        ImGui::EndChild();

        //-------------------------------------------------

        ImGui::Spacing();

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

        StartFrame();

        m_Renderer.OnRender();
        if (m_ShowMenu) m_Renderer.OnImGuiRender();

        EndFrame();
    }
}

void Application::OnEvent(Event& e) {
   EventType type = e.getEventType();

   switch(type) {
        case EventType::WindowResize: 
        {
            WindowResizeEvent* ptr = dynamic_cast<WindowResizeEvent*>(&e);
            m_Renderer.OnWindowResize(ptr->getWidth(), ptr->getHeight());
            break; 
        }
        case EventType::KeyPressed: 
        {
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
        case EventType::KeyReleased: 
        {
            if (!m_ShowMenu) {
                KeyReleasedEvent* ptr = dynamic_cast<KeyReleasedEvent*>(&e);
                m_Renderer.OnKeyReleased(ptr->getKeycode());
            }
            break;
        }
        case EventType::MouseMoved: 
        {
            if (!m_ShowMenu) {
                MouseMovedEvent* ptr = dynamic_cast<MouseMovedEvent*>(&e);
                m_Renderer.OnMouseMoved(ptr->getX(), ptr->getY());
            }
            break;
        }
        case EventType::MousePressed:
        {
            //This needs to be called when menu is open
            MousePressedEvent* ptr = dynamic_cast<MousePressedEvent*>(&e);
            m_Renderer.OnMousePressed(ptr->getButton(), ptr->getMods());
            break;
        }
   }
}

