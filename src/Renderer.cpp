#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height)
    , m_Aspect(float(m_WindowWidth) / float(m_WindowHeight))
    , m_InvAspect(1.0/m_Aspect)
{}

Renderer::~Renderer() {}

void Renderer::Init(StartSettings settings) {
    m_Clipmap.Init(settings.Subdivisions, settings.LodLevels);
    m_Map.Init(settings.HeightRes, settings.ShadowRes, settings.WrapType);

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

    //Return to normal rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnUpdate(float deltatime) {
    //Update camera
    glm::vec3 prev_pos3 = m_Camera.getPos();
    glm::vec2 prev_pos2 = { prev_pos3.x, prev_pos3.z };

    m_Camera.Update(deltatime);

    glm::vec3 curr_pos3 = m_Camera.getPos();
    glm::vec2 curr_pos2 = { curr_pos3.x, curr_pos3.z };

    glm::mat4 proj = m_Camera.getProjMatrix(m_Aspect);
    glm::mat4 view = m_Camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj * view * model;

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
    const float scale_y  = m_Map.getScaleSettings().ScaleY;

    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_Wireframe) {
        m_TerrainRenderer.PrepareWireframe(m_MVP, m_Camera, m_Map);

        m_Clipmap.BindAndDraw(m_Camera, scale_y);
    }

    else {
        //Render Clipmap (Terrain)
        m_TerrainRenderer.PrepareShaded(m_MVP, m_Camera, m_Map, m_Material, m_SkyRenderer);

        m_Clipmap.BindAndDraw(m_Camera, scale_y);

        //Render Sky
        m_SkyRenderer.Render(m_Camera.getFront(), m_Camera.getSettings().Fov, m_InvAspect);
    }
}

void Renderer::OnImGuiRender() {
    //-----Menu bar
    if (ImGui::BeginMainMenuBar()) {
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
            if (ImGui::MenuItem("Terrain"))
                m_ShowTerrainMenu = !m_ShowTerrainMenu;

            if (ImGui::MenuItem("Background"))
                m_ShowBackgroundMenu = !m_ShowBackgroundMenu;

            if (ImGui::MenuItem("Lighting"))
                m_ShowLightMenu = !m_ShowLightMenu;

            if (ImGui::MenuItem("Shadows"))
                m_ShowShadowMenu = !m_ShowShadowMenu;

            if (ImGui::MenuItem("Camera"))
                m_ShowCamMenu = !m_ShowCamMenu;

            if (ImGui::MenuItem("Material"))
                m_ShowMaterialMenu = !m_ShowMaterialMenu;

            if (ImGui::MenuItem("Material Map"))
                m_ShowMapMaterialMenu = !m_ShowMapMaterialMenu;

            if (ImGui::MenuItem("Sky"))
                m_ShowSkyMenu = !m_ShowSkyMenu;

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    //-----Dockspace
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

    //-----Windows
    if (m_ShowBackgroundMenu) {
        ImGui::Begin("Background", &m_ShowBackgroundMenu);
        ImGui::Columns(2, "###col");
        ImGuiUtils::ColorEdit3("##ClearColor", m_ClearColor);
        ImGui::Columns(1, "###col");
        ImGui::End();
    }
    
    if (m_ShowCamMenu)
        m_Camera.OnImGui(m_ShowCamMenu);
    
    if (m_ShowTerrainMenu)
        m_Map.ImGuiTerrain(m_ShowTerrainMenu, m_TerrainRenderer.DoShadows());

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

void Renderer::RestartMouse() {
    m_Camera.setMouseInit(true);
}
