//Temporary, to use sscanf with MSVC:
#define _CRT_SECURE_NO_WARNINGS

#include "Renderer.h"
#include "Keycodes.h"
#include "Profiler.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include <iostream>

Renderer::Renderer(uint32_t width, uint32_t height)
    : m_WindowWidth(width), m_WindowHeight(height)
    , m_Aspect(float(m_WindowWidth) / float(m_WindowHeight))
    , m_InvAspect(1.0f / m_Aspect)
    , m_Framebuffer(m_ResourceManager)
    , m_Map(m_ResourceManager)
    , m_Material(m_ResourceManager)
    , m_MaterialMap(m_ResourceManager, m_Map)
    , m_SkyRenderer(m_ResourceManager, m_Camera, m_Map)
    , m_TerrainRenderer(m_ResourceManager, m_Camera, m_Map, m_Material, m_MaterialMap, m_SkyRenderer)
    , m_GrassRenderer(m_ResourceManager, m_Camera, m_Map, m_Material, m_SkyRenderer)
    , m_PostProcessor(m_ResourceManager, m_Framebuffer)
{
    //Bind (de)serialization callbacks
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

    m_Serializer.RegisterLoadCallback("MaterialMap Editor",
        std::bind(&MaterialMapGenerator::OnDeserialize, &m_MaterialMap, std::placeholders::_1)
    );

    m_Serializer.RegisterSaveCallback("MaterialMap Editor",
        std::bind(&MaterialMapGenerator::OnSerialize, &m_MaterialMap, std::placeholders::_1)
    );

    //Initialize present shader
    m_PresentShader = m_ResourceManager.RequestVertFragShader(
        "res/shaders/present.vert", "res/shaders/present.frag"
    );
}

Renderer::~Renderer() {}

void Renderer::Init(StartSettings settings)
{
    m_TerrainRenderer.Init(settings.Subdivisions, settings.LodLevels);
    m_Map.Init(settings.HeightRes, settings.ShadowRes, settings.WrapType);
    m_Material.Init(settings.MaterialRes);
    m_MaterialMap.Init(settings.HeightRes, settings.WrapType);

    if (settings.IncludeGrass)
    {
        m_IncludeGrass = true;
        m_GrassRenderer.Init();
        m_GrassRenderer.RequestGeometryUpdate();
    }

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

    //Update Material Map
    m_MaterialMap.OnUpdate();

    //Initial geometry displacement
    m_TerrainRenderer.Update();

    //Framebuffer setup
    m_InternalResScale = settings.InternalResScale;

    m_InternalWidth  = static_cast<int>(m_InternalResScale * static_cast<float>(m_WindowWidth));
    m_InternalHeight = static_cast<int>(m_InternalResScale * static_cast<float>(m_WindowHeight));

    Texture2DSpec framebuffer_spec{
        m_InternalWidth, m_InternalHeight, GL_RGBA8, GL_RGBA,
        GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
        GL_MIRRORED_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Framebuffer.Initialize(framebuffer_spec);

    m_PostProcessor.Init(m_InternalHeight, m_InternalHeight);

    //Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::OnUpdate(float deltatime)
{
    ProfilerCPUEvent we("Renderer::OnUpdate");

    if (m_ResizeFramebuffer)
    {
        m_Framebuffer.Resize(m_InternalWidth, m_InternalHeight);
        m_PostProcessor.ResizeBuffers(m_InternalWidth, m_InternalHeight);

        m_ResizeFramebuffer = false;
    }

    if (m_Map.GeometryShouldUpdate())
    {
        m_MaterialMap.RequestUpdate();
        m_TerrainRenderer.RequestFullUpdate();
        m_GrassRenderer.RequestGeometryUpdate();
    }

    if (m_SkyRenderer.SunDirChanged() && m_TerrainRenderer.DoShadows())
        m_Map.RequestShadowUpdate();

    m_ResourceManager.OnUpdate();

    m_Camera.Update(m_Aspect, deltatime);

    m_Map.Update(m_SkyRenderer.getSunDir());

    m_MaterialMap.OnUpdate();

    if (m_IncludeGrass)
        m_GrassRenderer.OnUpdate(deltatime);

    m_SkyRenderer.Update(m_TerrainRenderer.DoFog());

    m_TerrainRenderer.Update();
}

void Renderer::OnRender()
{
    ProfilerCPUEvent we("Renderer::OnRender");

    const glm::vec3 clear_color = m_TerrainRenderer.getClearColor();

    if (m_Wireframe)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_WindowWidth, m_WindowHeight);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        m_TerrainRenderer.RenderWireframe();
    }

    else
    {
        //Initial rendering to framebuffer
        m_Framebuffer.BindFBO();
        glViewport(0, 0, m_InternalWidth, m_InternalHeight);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        m_TerrainRenderer.RenderShaded();

        if (m_IncludeGrass)
            m_GrassRenderer.Render();

        m_SkyRenderer.Render();

        m_Framebuffer.RequestPreviewUpdate();

        //Post processing
        m_PostProcessor.OnRender();

        //Presenting results to the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_WindowWidth, m_WindowHeight);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        m_PostProcessor.BindOutput(0);

        m_PresentShader->Bind();
        m_PresentShader->setUniformSampler2D("framebuffer", 0);

        m_Quad.Draw();
    }
}

void Renderer::OnImGuiRender()
{
    ProfilerCPUEvent we("Renderer::OnImGuiRender");

    //-----Menu bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
                m_Serializer.TriggerSave();

            if (ImGui::MenuItem("Load"))
                m_Serializer.TriggerLoad();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Shaded"))
            {
                m_Wireframe = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            if (ImGui::MenuItem("Wireframe"))
            {
                m_Wireframe = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem(LOFI_ICONS_TERRAIN     "Terrain",      NULL, &m_ShowTerrainMenu);
            ImGui::MenuItem(LOFI_ICONS_LIGHTING    "Lighting",     NULL, &m_ShowLightMenu);
            ImGui::MenuItem(LOFI_ICONS_SHADOW      "Shadows",      NULL, &m_ShowShadowMenu);
            ImGui::MenuItem(LOFI_ICONS_CAMERA      "Camera",       NULL, &m_ShowCamMenu);
            ImGui::MenuItem(LOFI_ICONS_MATERIAL    "Material",     NULL, &m_ShowMaterialMenu);
            ImGui::MenuItem(LOFI_ICONS_MATERIALMAP "Material Map", NULL, &m_ShowMapMaterialMenu);
            ImGui::MenuItem(LOFI_ICONS_SKY         "Sky",          NULL, &m_ShowSkyMenu);
            ImGui::MenuItem(LOFI_ICONS_POSTFX      "PostFX",       NULL, &m_ShowPostMenu);

            if (m_IncludeGrass)
                ImGui::MenuItem(LOFI_ICONS_GRASS   "Grass",        NULL, &m_ShowGrassMenu);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Reload Shaders"))
                m_ResourceManager.ReloadShaders();

            ImGui::MenuItem("Show Texture Browser", NULL, &m_ShowTexBrowser);
            ImGui::MenuItem("Show Profiler", NULL, &m_ShowProfiler);
            ImGui::MenuItem("Show Frustum Culling", NULL, &m_ShowCulling);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Welcome Popup"))
                m_ShowHelpPopup = true;

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

    if (m_IncludeGrass && m_ShowGrassMenu)
        m_GrassRenderer.OnImGui(m_ShowGrassMenu);

    if (m_ShowMapMaterialMenu)
        m_MaterialMap.OnImGui(m_ShowMapMaterialMenu);

    if(m_ShowShadowMenu)
        m_Map.ImGuiShadowmap(m_ShowShadowMenu, m_TerrainRenderer.DoShadows());

    if (m_ShowMaterialMenu)
        m_Material.OnImGui(m_ShowMaterialMenu);

    if (m_ShowLightMenu)
        m_TerrainRenderer.OnImGui(m_ShowLightMenu);

    if (m_ShowSkyMenu)
        m_SkyRenderer.OnImGui(m_ShowSkyMenu);

    if (m_ShowPostMenu)
        m_PostProcessor.OnImGui(m_ShowPostMenu);

    if (m_ShowTexBrowser)
        m_ResourceManager.DrawTextureBrowser(m_ShowTexBrowser);

    if (m_ShowProfiler)
        Profiler::OnImGui(m_ShowProfiler);

    if (m_ShowCulling)
        m_TerrainRenderer.OnImGuiDebugCulling(m_ShowCulling);

    //-----Help popup

    const std::string popup_name{ "Welcome to LofiLandscapes!" };

    if (m_ShowHelpPopup)
        ImGui::OpenPopup(popup_name.c_str());

    ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal(popup_name.c_str(), &m_ShowHelpPopup)) {

        ImGui::Bullet();
        ImGui::TextWrapped("Use escape to close/re-open the gui");
        ImGui::Bullet();
        ImGui::TextWrapped("When gui is closed:");
        ImGui::TextWrapped("        Use WASD to move around");
        ImGui::TextWrapped("        Use mouse/touchpad to look around");
        ImGui::Bullet();
        ImGui::TextWrapped("When using a touchpad, if you can't turn while moving, try disabling \"palm check\" or other similar settings");
        ImGui::Bullet();
        ImGui::TextWrapped("Use File, Open to see example worlds");
        ImGui::Bullet();
        ImGui::TextWrapped("Open things from Windows tab to see what's possible");

        ImGui::EndPopup();
    }
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
    m_WindowWidth = width;
    m_WindowHeight = height;

    m_InternalWidth  = static_cast<uint32_t>(m_InternalResScale * static_cast<float>(width));
    m_InternalHeight = static_cast<uint32_t>(m_InternalResScale * static_cast<float>(height));

    m_Aspect = float(m_WindowWidth) / float(m_WindowHeight);
    m_InvAspect = 1.0f / m_Aspect;

    m_ResizeFramebuffer = true;
}

void Renderer::OnKeyPressed(int keycode, bool repeat)
{
    m_Camera.OnKeyPressed(keycode, repeat);
}

void Renderer::OnKeyReleased(int keycode)
{
    m_Camera.OnKeyReleased(keycode);
}

void Renderer::OnMouseMoved(float x, float y)
{
    m_Camera.OnMouseMoved(x, y, m_WindowWidth, m_WindowHeight);
}

void Renderer::RestartMouse()
{
    m_Camera.setMouseInit(true);
}

void Renderer::InitImGuiIniHandler()
{
    auto MyUserData_ReadOpen = [](ImGuiContext* /*ctx*/, ImGuiSettingsHandler* /*handler*/, const char* /*name*/)
    {
        return (void*)1;
    };

    auto MyUserData_ReadLine = [](ImGuiContext* /*ctx*/, ImGuiSettingsHandler* handler, void* /*entry*/, const char* line)
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
        if (CheckLine("ShowPostMenu=%d\n"))        r->m_ShowPostMenu        = bool(value);
        if (CheckLine("ShowHelp=%d\n"))            r->m_ShowHelpPopup       = bool(value);
    };

    auto MyUserData_WriteAll = [](ImGuiContext* /*ctx*/, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf)
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
        out_buf->appendf("ShowPostMenu=%d\n", r->m_ShowPostMenu);
        out_buf->appendf("ShowHelp=%d\n", r->m_ShowHelpPopup);
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
