#pragma once

#include "Shader.h"

#include "Camera.h"

#include "MapGenerator.h"
#include "MaterialGenerator.h"
#include "SkyRenderer.h"

class TerrainRenderer {
public:
    TerrainRenderer();
    ~TerrainRenderer();

    void PrepareWireframe(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map);
    void PrepareShaded(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map, 
                       const MaterialGenerator& material, const SkyRenderer& sky);

    void OnImGui(bool& open);

    bool DoShadows() { return m_Shadows; }
    bool DoFog() { return m_Fog; }

private:

    float m_SunCol[3] = { 0.90f, 0.85f, 0.70f };

    float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;

    float m_TilingFactor = 32.0f, m_NormalStrength = 0.25f;

    bool m_Shadows = true;
    bool m_Materials = true, m_FixTiling = true;
    bool m_Fog = false;

    Shader m_ShadedShader, m_WireframeShader;
};