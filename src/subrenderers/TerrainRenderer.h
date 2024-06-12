#pragma once

#include "Shader.h"

#include "Camera.h"

#include "MapGenerator.h"
#include "MaterialGenerator.h"
#include "MaterialMapGenerator.h"
#include "SkyRenderer.h"
#include "Clipmap.h"

#include "ResourceManager.h"

class TerrainRenderer {
public:
    TerrainRenderer(ResourceManager& manager, const PerspectiveCamera& cam,
                    const MapGenerator& map, const MaterialGenerator& material,
                    const MaterialMapGenerator& material_map, const SkyRenderer& sky);
    ~TerrainRenderer();

    void Init(uint32_t subdivisions, uint32_t levels);

    void Update();
    void RequestFullUpdate();

    void RenderWireframe();
    void RenderShaded();

    void OnImGui(bool& open);
    void OnImGuiDebugCulling(bool& open);

    bool DoShadows() { return m_Shadows; }
    bool DoFog() { return m_Fog; }

    glm::vec3 getClearColor() const { return m_ClearColor; }

private:
    //Settings
    glm::vec3 m_ClearColor{ 0.0f, 0.0f, 0.0f };

    float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;

    float m_TilingFactor = 128.0f, m_NormalStrength = 0.333f;

    bool m_Shadows = true;
    bool m_Materials = true, m_FixTiling = true;
    bool m_Fog = true;

    //External handles
    ResourceManager& m_ResourceManager;

    const PerspectiveCamera& m_Camera;
    const MapGenerator& m_Map;
    const MaterialGenerator& m_Material;
    const MaterialMapGenerator& m_MaterialMap;
    const SkyRenderer& m_Sky;

    //Private resources
    bool m_UpdateAll = true;

    std::shared_ptr<VertFragShader> m_ShadedShader, m_WireframeShader;
    std::shared_ptr<ComputeShader> m_DisplaceShader;

    //Binding ids for shader buffers
    static constexpr uint32_t m_VertBinding = 1;
    //static constexpr uint32_t m_SSBOBinding = 2;
    static constexpr uint32_t m_UBOBinding = 2;

    Clipmap m_Clipmap;
};
