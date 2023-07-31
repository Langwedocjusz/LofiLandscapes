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

	enum UpdateFlags {
		None    = 0,
		Raycast = (1 << 0),
		Noise   = (1 << 1)
	};

	int m_UpdateFlags = Raycast | Noise;

	bool m_RenderGrass = false;

	float m_GrassHeight = 0.5f, m_Tiling = 2.75f, m_MaxDepth = 1.0f, m_NoiseTiling = 0.31f;

	int m_LodLevels = 2;

	int m_NoiseScale = 5, m_Octaves = 3;
	float m_NoiseStrength = 1.04f, m_Sway = 0.09f;
	float m_AOMin = 0.12f, m_AOMax = 0.59f;

	float m_Time = 0.0f;
	glm::vec2 m_ScrollingVelocity = glm::vec2(0.2f, 0.2f);

	float m_ViewAngle = 1.03f;
	float m_BaseWidth = 0.08f, m_Slant = 0.96f;

	std::shared_ptr<ComputeShader> m_RaycastShader;
	std::shared_ptr<ComputeShader> m_NoiseGenerator;

	std::shared_ptr<Texture3D> m_RaycastResult;
	std::shared_ptr<Texture2D> m_Noise;

	std::shared_ptr<VertFragShader> m_PresentShader;

	ResourceManager& m_ResourceManager;
};