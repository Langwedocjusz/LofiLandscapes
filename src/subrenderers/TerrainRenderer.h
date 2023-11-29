#pragma once

#include "Shader.h"

#include "Camera.h"

#include "MapGenerator.h"
#include "MaterialGenerator.h"
#include "SkyRenderer.h"
#include "Clipmap.h"

#include "ResourceManager.h"

class TerrainRenderer {
public:
    TerrainRenderer(ResourceManager& manager, const PerspectiveCamera& cam,
                    const MapGenerator& map, const MaterialGenerator& material,
                    const SkyRenderer& sky);
    ~TerrainRenderer();

    void Init(uint32_t subdivisions, uint32_t levels);

    void Update();
    void RequestFullUpdate();

    void RenderWireframe();
    void RenderShaded();

    void OnImGui(bool& open);

    bool DoShadows() { return m_Shadows; }
    bool DoFog() { return m_Fog; }

    glm::vec3 getClearColor() const { return m_ClearColor; }

private:
    //Settings
    glm::vec3 m_ClearColor{ 0.0f, 0.0f, 0.0f };

    float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;

    float m_TilingFactor = 128.0f, m_NormalStrength = 0.25f;

    bool m_Shadows = true;
    bool m_Materials = true, m_FixTiling = true;
    bool m_Fog = true;

    //Private resources
    bool m_UpdateAll = true;

    std::shared_ptr<VertFragShader> m_ShadedShader, m_WireframeShader;
    std::shared_ptr<ComputeShader> m_DisplaceShader;

    Clipmap m_Clipmap;

    //External handles
    const PerspectiveCamera& m_Camera;
    const MapGenerator& m_Map;
    const MaterialGenerator& m_Material;
    const SkyRenderer& m_Sky;

    ResourceManager& m_ResourceManager;
};