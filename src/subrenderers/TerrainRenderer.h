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
                    const Clipmap& clipmap, const SkyRenderer& sky);
    ~TerrainRenderer();

    void RenderWireframe();
    void RenderShaded();

    void OnImGui(bool& open);

    bool DoShadows() { return m_Shadows; }
    bool DoFog() { return m_Fog; }

private:
    float m_ClearColor[3] = { 0.0f, 0.0f, 0.0f };

    glm::vec3 m_SunCol{ 0.90f, 0.85f, 0.70f };

    float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;

    float m_TilingFactor = 128.0f, m_NormalStrength = 0.25f;

    bool m_Shadows = true;
    bool m_Materials = true, m_FixTiling = true;
    bool m_Fog = true;

    std::shared_ptr<VertFragShader> m_ShadedShader, m_WireframeShader;

    //External handles
    const PerspectiveCamera& m_Camera;
    const MapGenerator& m_Map;
    const MaterialGenerator& m_Material;
    const Clipmap& m_Clipmap;
    const SkyRenderer& m_Sky;

    ResourceManager& m_ResourceManager;
};