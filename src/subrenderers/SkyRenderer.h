#pragma once

#include "Shader.h"
#include "Texture.h"
#include "GLUtils.h"
#include "ResourceManager.h"
#include "MapGenerator.h"

#include "Camera.h"

class SkyRenderer {
public:
	SkyRenderer(ResourceManager& manager, const PerspectiveCamera& cam, const MapGenerator& map);

	void OnImGui(bool& open);
	void Update(bool aerial);
	void Render();

	void BindSkyLUT(int id=0) const;
	void BindIrradiance(int id=0) const;
	void BindPrefiltered(int id=0) const;
	void BindAerial(int id=0) const;

	glm::vec3 getSunDir() const { return m_SunDir; }
	glm::vec3 getSunCol() const;

	bool SunDirChanged() const { return m_SunDirChanged; }

	float getAerialDistScale() const { return m_AerialDistRead; }
private:
	void Init();

	enum SkyUpdateFlags {
		None          =  0,
		Transmittance = (1 << 0),
		MultiScatter  = (1 << 1),
		SkyView       = (1 << 2),
		SunColor      = (1 << 3)
	};

	void UpdateTrans();
	void UpdateMulti();
	void UpdateSky();
	void UpdateAerial();
	void UpdateAerialWithShadows();

	//This function exactly mirrors transmittance calculation from the 
	//transmittance LUT, but only for the sun direction
	void CalculateSunTransmittance();

	int m_UpdateFlags = None;

	//Sky/Atmosphere parameters
	glm::vec3 m_SunCol{ 1.0f, 1.0f, 1.0f };
	glm::vec3 m_SunTrans{1.0f, 1.0f, 1.0f};

	float m_TransInfluence = 1.0f;
	float m_TransCurve = 2.0f;

	bool m_UseSunTransmittance = true;

	float m_Height = 300.0f; //in meters
	glm::vec3 m_GroundAlbedo = glm::vec3(0.25f, 0.25f, 0.25f);

	float m_Phi = 1.032f, m_Theta = 1.050f;
	glm::vec3 m_SunDir;
	bool m_SunDirChanged = false;

	float m_Brightness = 6.0f;
	float m_IBLOversaturation = 1.4f;

	//Aerial
	float m_AerialBrightness = 47.0f;
	float m_AerialDistWrite = 10.0f;
	float m_AerialDistRead = 1.0f;

	bool m_AerialMultiscatter = true;
	float m_AerialMultiWeight = 0.092f;

	bool m_AerialShadows = false;
	bool m_ShowShadows = true;

	//Private resources
	std::shared_ptr<Texture2D> m_TransLUT, m_MultiLUT, m_SkyLUT;
	std::shared_ptr<Texture3D> m_AerialLUT;
	std::shared_ptr<Texture3D> m_ScatterVolume, m_ShadowVolume;
	std::shared_ptr<ComputeShader> m_TransShader, m_MultiShader, m_SkyShader;

	std::shared_ptr<ComputeShader> m_AerialShader;
	std::shared_ptr<ComputeShader> m_AScatterShader, m_AShadowShader, m_ARaymarchShader;

	std::shared_ptr<Cubemap> m_IrradianceMap, m_PrefilteredMap;
	std::shared_ptr<ComputeShader> m_IrradianceShader, m_PrefilteredShader;

	Quad m_Quad;
	std::shared_ptr<VertFragShader> m_FinalShader;

	//External handles
	const PerspectiveCamera& m_Camera;
	const MapGenerator& m_Map;

	ResourceManager& m_ResourceManager;
};