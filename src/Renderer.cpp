#include "Renderer.h"
#include "Keycodes.h"
#include "Profiler.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height)
    , m_Aspect(float(m_WindowWidth) / float(m_WindowHeight))
    , m_InvAspect(1.0/m_Aspect)
    , m_TerrainRenderer(m_ResourceManager)
    , m_GrassRenderer(m_ResourceManager)
    , m_Clipmap(m_ResourceManager)
    , m_Map(m_ResourceManager)
    , m_Material(m_ResourceManager)
    , m_SkyRenderer(m_ResourceManager)
{
    m_Serializer.RegisterLoadCallback("Terrain Editor",
        std::bind(&MapGenerator::OnDeserialize, &m_Map, std::placeholders::_1)
    );

    m_Serializer.RegisterSaveCallback("Terrain Editor",
        std::bind(&MapGenerator::OnSerialize, &m_Map, std::placeholders::_1)
    );

    m_Serializer.RegisterLoadCallback("Material Editor",
        std::bind(&MaterialGenerator::OnDeserialize, &m_Material, std::placeholders::_1)
    );

    m_Serializer.RegisterSaveCallback("Material Editor", 
        std::bind(&MaterialGenerator::OnSerialize, &m_Material, std::placeholders::_1)
    );
}

Renderer::~Renderer() {}

void Renderer::Init(StartSettings settings) {
    m_Clipmap.Init(settings.Subdivisions, settings.LodLevels);
    m_Map.Init(settings.HeightRes, settings.ShadowRes, settings.WrapType);
    m_Material.Init(settings.MaterialRes);

    m_GrassRenderer.Init();

    glEnable(GL_DEPTH_TEST);
    //Depth function to allow sky with maximal depth (1.0)
    //being rendered after all geometry
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //Backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    //Seamless cubemaps
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //Update Maps
    m_Map.Update(m_SkyRenderer.getSunDir());

    //Update Material
    m_Material.Update();

    //Initial geometry displacement
    m_Map.BindHeightmap();

    glm::vec2 pos{ m_Camera.getPos().x, m_Camera.getPos().z };

    m_Clipmap.DisplaceVertices(
        m_Map.getScaleSettings().ScaleXZ,
        m_Map.getScaleSettings().ScaleY,
        pos
    );
}

void Renderer::OnUpdate(float deltatime) {
    ProfilerCPUEvent we("Renderer::OnUpdate");

    m_ResourceManager.OnUpdate();

    m_GrassRenderer.OnUpdate(deltatime);

    //Update camera
    glm::vec3 prev_pos3 = m_Camera.getPos();
    glm::vec2 prev_pos2 = { prev_pos3.x, prev_pos3.z };

    m_Camera.Update(m_Aspect, deltatime);

    glm::vec3 curr_pos3 = m_Camera.getPos();
    glm::vec2 curr_pos2 = { curr_pos3.x, curr_pos3.z };


    glm::mat4 proj_view = m_Camera.getViewProjMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj_view * model;

    //Update sky
    m_SkyRenderer.Update(m_Camera, m_InvAspect, m_TerrainRenderer.DoFog());

    //Update clipmap geometry if camera moved
    m_Map.BindHeightmap();

    m_Clipmap.DisplaceVertices(
        m_Map.getScaleSettings().ScaleXZ,
        m_Map.getScaleSettings().ScaleY,
        curr_pos2, prev_pos2
    );
}

void Renderer::OnRender() {
    ProfilerCPUEvent we("Renderer::OnRender");

    const float scale_y  = m_Map.getScaleSettings().ScaleY;

    if (m_Wireframe) 
    {
        m_TerrainRenderer.RenderWireframe(m_MVP, m_Camera, m_Map, m_Clipmap);
    }

    else 
    {
        m_TerrainRenderer.RenderShaded(m_MVP, m_Camera, m_Map, m_Material, m_SkyRenderer, m_Clipmap);

        m_GrassRenderer.Render(m_MVP, m_Camera, m_Map, m_Material, m_SkyRenderer, m_Clipmap);

        m_SkyRenderer.Render(m_Camera.getFront(), m_Camera.getFov(), m_InvAspect);
    }
}

void Renderer::OnImGuiRender() {
    ProfilerCPUEvent we("Renderer::OnImGuiRender");

    //-----Menu bar
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Save")) {
                m_Serializer.TriggerSave();
            }

            if (ImGui::MenuItem("Load")) {
                m_Serializer.TriggerLoad();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Shaded")) {
                m_Wireframe = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            if (ImGui::MenuItem("Wireframe")) {
                m_Wireframe = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {

            ImGui::MenuItem(LOFI_ICONS_TERRAIN     "Terrain",      NULL, &m_ShowTerrainMenu);
            ImGui::MenuItem(LOFI_ICONS_GRASS       "Grass",        NULL, &m_ShowGrassMenu);
            ImGui::MenuItem(LOFI_ICONS_LIGHTING    "Lighting",     NULL, &m_ShowLightMenu);
            ImGui::MenuItem(LOFI_ICONS_SHADOW      "Shadows",      NULL, &m_ShowShadowMenu);
            ImGui::MenuItem(LOFI_ICONS_CAMERA      "Camera",       NULL, &m_ShowCamMenu);
            ImGui::MenuItem(LOFI_ICONS_MATERIAL    "Material",     NULL, &m_ShowMaterialMenu);
            ImGui::MenuItem(LOFI_ICONS_MATERIALMAP "Material Map", NULL, &m_ShowMapMaterialMenu);
            ImGui::MenuItem(LOFI_ICONS_SKY         "Sky",          NULL, &m_ShowSkyMenu);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {

            if (ImGui::MenuItem("Reload Shaders"))
                m_ResourceManager.ReloadShaders();

            ImGui::MenuItem("Show Texture Browser", NULL, &m_ShowTexBrowser);

            ImGui::MenuItem("Show Profiler", NULL, &m_ShowProfiler);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    //-----Dockspace
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

    //Always called
    m_Serializer.OnImGui();

    //-----Windows
    
    if (m_ShowCamMenu)
        m_Camera.OnImGui(m_ShowCamMenu);
    
    if (m_ShowTerrainMenu)
        m_Map.ImGuiTerrain(m_ShowTerrainMenu, m_TerrainRenderer.DoShadows());

    if (m_ShowGrassMenu)
        m_GrassRenderer.OnImGui(m_ShowGrassMenu);

    if (m_ShowMapMaterialMenu)
        m_Map.ImGuiMaterials(m_ShowMapMaterialMenu);

    bool update_geo = m_Map.GeometryShouldUpdate();

    if(m_ShowShadowMenu)
        m_Map.ImGuiShadowmap(m_ShowShadowMenu, m_TerrainRenderer.DoShadows());

    if (m_ShowMaterialMenu)
        m_Material.OnImGui(m_ShowMaterialMenu);

    if (m_ShowLightMenu) {
        bool shadows = m_TerrainRenderer.DoShadows();

        m_TerrainRenderer.OnImGui(m_ShowLightMenu);

        if (shadows != m_TerrainRenderer.DoShadows()) {
            if (m_TerrainRenderer.DoShadows()) 
                m_Map.RequestShadowUpdate();
        }
    }

    if (m_ShowSkyMenu) {
        glm::vec3 sun_dir = m_SkyRenderer.getSunDir();

        m_SkyRenderer.OnImGui(m_ShowSkyMenu);

        if (sun_dir != m_SkyRenderer.getSunDir() && m_TerrainRenderer.DoShadows())
            m_Map.RequestShadowUpdate();
    }

    if (m_ShowTexBrowser)
        m_ResourceManager.DrawTextureBrowser(m_ShowTexBrowser);

    if (m_ShowProfiler)
        Profiler::OnImGui(m_ShowProfiler);

    //Update Maps
    m_Map.Update(m_SkyRenderer.getSunDir());

    //Update geometry if heightmap/scale changed
    if (update_geo) {
        m_Map.BindHeightmap();

        glm::vec2 pos{ m_Camera.getPos().x, m_Camera.getPos().z };

        m_Clipmap.DisplaceVertices(
            m_Map.getScaleSettings().ScaleXZ,
            m_Map.getScaleSettings().ScaleY,
            pos
        );
    }
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {
    m_WindowWidth = width;
    m_WindowHeight = height;

    m_Aspect = float(m_WindowWidth) / float(m_WindowHeight);
    m_InvAspect = 1.0 / m_Aspect;

    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    m_Camera.OnKeyPressed(keycode, repeat);
}

void Renderer::OnKeyReleased(int keycode) {
    m_Camera.OnKeyReleased(keycode);
}

void Renderer::OnMouseMoved(float x, float y) {
    m_Camera.OnMouseMoved(x, y, m_WindowWidth, m_WindowHeight, m_Aspect);
}

void Renderer::OnMousePressed(int button, int mods) {
    //std::cout << button << mods << '\n';
}

void Renderer::RestartMouse() {
    m_Camera.setMouseInit(true);
}

void Renderer::InitImGuiIniHandler() {
    auto MyUserData_ReadOpen = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name)
    {
        return (void*)1;
    };

    auto MyUserData_ReadLine = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line)
    {
        Renderer* r = (Renderer*)handler->UserData;
        int value;

        auto CheckLine = [&line, &value](const char* fmt)
        {
            return sscanf(line, fmt, &value) == 1;
        };

        if (CheckLine("ShowTerrainMenu=%d\n"))     r->m_ShowTerrainMenu     = bool(value);
        if (CheckLine("ShowGrassMenu=%d\n"))       r->m_ShowGrassMenu       = bool(value);
        if (CheckLine("ShowLightMenu=%d\n"))       r->m_ShowLightMenu       = bool(value);
        if (CheckLine("ShowShadowMenu=%d\n"))      r->m_ShowShadowMenu      = bool(value);
        if (CheckLine("ShowCamMenu=%d\n"))         r->m_ShowCamMenu         = bool(value);
        if (CheckLine("ShowMaterialMenu=%d\n"))    r->m_ShowMaterialMenu    = bool(value);
        if (CheckLine("ShowSkyMenu=%d\n"))         r->m_ShowSkyMenu         = bool(value);
        if (CheckLine("ShowMapMaterialMenu=%d\n")) r->m_ShowMapMaterialMenu = bool(value);
    };

    auto MyUserData_WriteAll = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
    {
        Renderer* r = (Renderer*)handler->UserData;

        out_buf->appendf("[%s][State]\n", handler->TypeName);
        out_buf->appendf("ShowTerrainMenu=%d\n", r->m_ShowTerrainMenu);
        out_buf->appendf("ShowGrassMenu=%d\n", r->m_ShowGrassMenu);
        out_buf->appendf("ShowLightMenu=%d\n", r->m_ShowLightMenu);
        out_buf->appendf("ShowShadowMenu=%d\n", r->m_ShowShadowMenu);
        out_buf->appendf("ShowCamMenu=%d\n", r->m_ShowCamMenu);
        out_buf->appendf("ShowMaterialMenu=%d\n", r->m_ShowMaterialMenu);
        out_buf->appendf("ShowSkyMenu=%d\n", r->m_ShowSkyMenu);
        out_buf->appendf("ShowMapMaterialMenu=%d\n", r->m_ShowMapMaterialMenu);
        out_buf->appendf("\n");
    };

    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "UserData";
    ini_handler.TypeHash = ImHashStr("UserData");
    ini_handler.ReadOpenFn = MyUserData_ReadOpen;
    ini_handler.ReadLineFn = MyUserData_ReadLine;
    ini_handler.WriteAllFn = MyUserData_WriteAll;
    ini_handler.UserData = this;

    ImGui::AddSettingsHandler(&ini_handler);
}