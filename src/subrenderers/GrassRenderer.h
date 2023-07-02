#pragma once

#include "ResourceManager.h"
#include "Camera.h"
#include "MapGenerator.h"
#include "MaterialGenerator.h"
#include "SkyRenderer.h"
#include "Clipmap.h"

class GrassRenderer{
public:
	GrassRenderer(ResourceManager& manager);

	void Init();
	void OnUpdate(float deltatime);
	void OnImGui(bool& open);

	void Render(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map,
		const MaterialGenerator& material, const SkyRenderer& sky, const Clipmap& clipmap);

private:
	//===Temporary - those values are doubled in TerrainRenderer=====================
	float m_SunCol[3] = { 0.90f, 0.85f, 0.70f };

	float m_SunStr = 2.0f, m_SkyDiff = 0.125f, m_SkySpec = 0.175f, m_RefStr = 0.100f;

	bool m_Shadows = true;
	//===============================================================================

	bool m_UpdateRaycast = true, m_RenderGrass = false;

	float m_GrassHeight = 0.5f, m_Tiling = 1.0f, m_MaxDepth = 1.0f, m_NoiseTiling = 1.0f;

	int m_LodLevels = 2;

	//glm::vec3 m_Slant = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_NoiseScale = 1.0f, m_NoiseStrength = 1.0f;

	float m_Time = 0.0f;
	glm::vec2 m_ScrollingVelocity = glm::vec2(1.0f, 0.0f);

	std::shared_ptr<ComputeShader> m_RaycastShader;
	std::shared_ptr<ComputeShader> m_NoiseGenerator;

	std::shared_ptr<Texture3D> m_RaycastResult;
	std::shared_ptr<Texture2D> m_Noise;

	std::shared_ptr<VertFragShader> m_PresentShader;

	ResourceManager& m_ResourceManager;
};