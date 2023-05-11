#pragma once

#include "Shader.h"
#include "GLUtils.h"
#include "ResourceManager.h"

#include "Camera.h"

class SkyRenderer {
public:
	SkyRenderer(ResourceManager& manager);

	void OnImGui(bool& open);
	void Update(const Camera& cam, float aspect, bool aerial);
	void Render(glm::vec3 cam_dir, float cam_fov, float aspect);

	void BindSkyLUT(int id=0) const;
	void BindIrradiance(int id=0) const;
	void BindPrefiltered(int id=0) const;
	void BindAerial(int id=0) const;

	glm::vec3 getSunDir() const { return m_SunDir; }
private:

	enum SkyUpdateFlags {
		None          =  0,
		Transmittance = (1 << 0),
		MultiScatter  = (1 << 1),
		SkyView       = (1 << 2)
	};

	void UpdateTrans();
	void UpdateMulti();
	void UpdateSky();
	void UpdateAerial(const Camera& cam, float aspect);

	int m_UpdateFlags = None;

	Texture m_TransLUT, m_MultiLUT, m_SkyLUT;
	Texture3d m_AerialLUT;
	std::shared_ptr<ComputeShader> m_TransShader, m_MultiShader, m_SkyShader, m_AerialShader;

	Cubemap m_IrradianceMap, m_PrefilteredMap;
	std::shared_ptr<ComputeShader> m_IrradianceShader, m_PrefilteredShader;

	Quad m_Quad;
	std::shared_ptr<VertFragShader> m_FinalShader;

	ResourceManager& m_ResourceManager;

	//Atmosphere parameters
	float m_Height = 300.0f; //in meters
	glm::vec3 m_GroundAlbedo = glm::vec3(0.25f, 0.25f, 0.25f);

	float m_Phi = 1.032f, m_Theta = 1.050f;
	glm::vec3 m_SunDir;

	float m_Brightness = 6.0f;
	float m_IBLOversaturation = 1.4f;

	//Aerial
	float m_AerialBrightness = 1.0f;
	float m_AerialDistscale = 1.0f;
};