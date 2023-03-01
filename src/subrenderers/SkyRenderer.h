#pragma once

#include "Shader.h"
#include "GLUtils.h"

enum class SkyUpdateFlags {
	None          =      0,
	Transmittance = (1<<0),
	MultiScatter  = (1<<1),
	SkyView       = (1<<2)
};

class SkyRenderer {
public:
	SkyRenderer();
	~SkyRenderer();

	void OnImGui(bool& open);
	void Update();
	void UpdateAerial(glm::vec3 front, glm::vec3 right,
		              glm::vec3 up, float cam_fov,
		              float aspect, float far);
	void Render(glm::vec3 cam_dir, float cam_fov, float aspect);

	void BindSkyLUT(int id=0);
	void BindIrradiance(int id=0);
	void BindPrefiltered(int id=0);
	void BindAerial(int id=0);

	glm::vec3 getSunDir() { return m_SunDir; }
private:
	void UpdateTrans();
	void UpdateMulti();
	void UpdateSky();

	Texture m_TransLUT, m_MultiLUT, m_SkyLUT;
	Texture3d m_AerialLUT;
	Shader m_TransShader, m_MultiShader, m_SkyShader, m_AerialShader;
	SkyUpdateFlags m_UpdateFlags;

	Cubemap m_IrradianceMap, m_PrefilteredMap;
	Shader m_IrradianceShader, m_PrefilteredShader;

	Quad m_Quad;
	Shader m_FinalShader;

	//Atmosphere parameters
	float m_Height = 300.0f; //in meters
	glm::vec3 m_GroundAlbedo = glm::vec3(0.25f, 0.25f, 0.25f);

	float m_Phi = 1.032f, m_Theta = 1.050f;
	glm::vec3 m_SunDir;

	float m_Brightness = 6.0f;

	//Aerial
	float m_AerialBrightness = 1.0f;
	float m_AerialDistscale = 1.0f;
};

SkyUpdateFlags operator|(SkyUpdateFlags x, SkyUpdateFlags y);
SkyUpdateFlags operator&(SkyUpdateFlags x, SkyUpdateFlags y);