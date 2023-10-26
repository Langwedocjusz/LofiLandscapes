#include "GrassRenderer.h"

#include "Profiler.h"

#include "glad/glad.h"
#include "imgui.h"

#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

GrassRenderer::GrassRenderer(ResourceManager& manager)
	: m_ResourceManager(manager)
{
	m_RaycastShader = m_ResourceManager.RequestComputeShader("res/shaders/grass/raycast.glsl");
	m_NoiseGenerator = m_ResourceManager.RequestComputeShader("res/shaders/grass/noise.glsl");

	m_PresentShader = m_ResourceManager.RequestVertFragShader(
		"res/shaders/grass/present.vert", 
		"res/shaders/grass/present.frag"
	);

	m_RaycastResult = m_ResourceManager.RequestTexture3D();
	m_Noise = m_ResourceManager.RequestTexture2D();
}

void GrassRenderer::Init()
{
	m_RaycastResult->Initialize(Texture3DSpec{
		128, 128, 16,
		GL_RGBA16F, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
		GL_REPEAT,
		{0.0f, 0.0f, 0.0f, 0.0f}
	});

	m_Noise->Initialize(Texture2DSpec{
		256, 256,
		GL_RGBA16F, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
		GL_REPEAT,
		{0.0f, 0.0f, 0.0f, 0.0f}
	});

	OnUpdate(0.0f);
	m_UpdateFlags = Noise;
}

void GrassRenderer::OnUpdate(float deltatime)
{
	m_Time += deltatime;
	if (m_Time > 1e3) m_Time = 0.0f;

	if ((m_UpdateFlags & Raycast) != None)
	{
		ProfilerGPUEvent we("Grass::Raycast");

		auto res_x = m_RaycastResult->getSpec().ResolutionX;
		auto res_y = m_RaycastResult->getSpec().ResolutionY;
		auto res_z = m_RaycastResult->getSpec().ResolutionZ;

		m_RaycastResult->BindImage(0, 0);

		m_RaycastShader->Bind();
		m_RaycastShader->setUniform1f("uViewAngle", m_ViewAngle);
		m_RaycastShader->setUniform1f("uSlant", m_Slant);
		m_RaycastShader->setUniform1f("uBaseWidth", m_BaseWidth);
		m_RaycastShader->setUniform1f("uNumBlades", static_cast<float>(m_NumBlades));

		m_RaycastShader->Dispatch(res_x, res_y, res_z);
		
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		//m_RaycastResult->Bind();
		//glGenerateMipmap(GL_TEXTURE_3D);

		m_ResourceManager.RequestPreviewUpdate(m_RaycastResult);
	}

	if ((m_UpdateFlags & Noise) != None)
	{
		ProfilerGPUEvent we("Grass::NoiseGen");

		auto res_x = m_Noise->getSpec().ResolutionX;
		auto res_y = m_Noise->getSpec().ResolutionY;

		m_Noise->BindImage(0, 0);

		m_NoiseGenerator->Bind();
		m_NoiseGenerator->setUniform1i("uScale", m_NoiseScale);
		m_NoiseGenerator->setUniform1i("uOctaves", m_Octaves);

		m_NoiseGenerator->Dispatch(res_x, res_y, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		m_Noise->Bind();
		glGenerateMipmap(GL_TEXTURE_2D);

		m_ResourceManager.RequestPreviewUpdate(m_Noise);
	}

	m_UpdateFlags = None;
}

void GrassRenderer::OnImGui(bool& open)
{
	ImGui::Begin(LOFI_ICONS_GRASS "Grass", &open, ImGuiWindowFlags_NoFocusOnAppearing);

	if (ImGui::CollapsingHeader("Precomputed raycast"))
	{
		ImGui::Columns(2, "###col");
		ImGuiUtils::ColSliderFloat("Slant", &m_Slant, 0.5f, 1.5f);
		ImGuiUtils::ColSliderFloat("Base Width", &m_BaseWidth, 0.01f, 0.1f);
		ImGuiUtils::ColSliderFloat("View Angle", &m_ViewAngle, 0.0f, 1.5f);
		ImGuiUtils::ColSliderInt("Num Blades", &m_NumBlades, 0, 10);
		ImGui::Columns(1, "###col");

		if (ImGuiUtils::ButtonCentered("Recalculate raycast"))
		{
			m_UpdateFlags |= Raycast;
		}
	}
	
	if (ImGui::CollapsingHeader("Noise"))
	{
		int tmp_scale = m_NoiseScale, tmp_octaves = m_Octaves;

		ImGui::Columns(2, "###col");
		ImGuiUtils::ColSliderInt("NoiseScale", &tmp_scale, 0, 10);
		ImGuiUtils::ColSliderInt("Octaves", &tmp_octaves, 0, 10);
		ImGui::Columns(1, "###col");

		if (tmp_scale != m_NoiseScale || tmp_octaves != m_Octaves)
		{
			m_NoiseScale = tmp_scale;
			m_Octaves = tmp_octaves;

			m_UpdateFlags |= Noise;
		}
	}

	if (ImGui::CollapsingHeader("Rendering"))
	{
		ImGui::Columns(2, "###col");

		ImGuiUtils::ColCheckbox("Render grass", &m_RenderGrass);

		ImGuiUtils::ColSliderInt("Lod Levels", &m_LodLevels, 0, 5);

		ImGuiUtils::ColSliderFloat("Height", &m_GrassHeight, 0.0f, 10.0f);
		ImGuiUtils::ColSliderFloat("Tiling", &m_Tiling, 0.0f, 20.0f);
		ImGuiUtils::ColSliderFloat("MaxDepth", &m_MaxDepth, 0.0f, 10.0f);

		ImGuiUtils::ColSliderFloat("NoiseTiling", &m_NoiseTiling, 0.0f, 1.0f);

		ImGuiUtils::ColSliderFloat("NoiseStrength", &m_NoiseStrength, 0.0f, 10.0f);
		ImGuiUtils::ColSliderFloat("Sway", &m_Sway, 0.0f, 1.0f);

		ImGuiUtils::ColDragFloat2("Velocity", glm::value_ptr(m_ScrollingVelocity));

		ImGuiUtils::ColSliderFloat("AO Min", &m_AOMin, 0.0f, 1.0f);
		ImGuiUtils::ColSliderFloat("AO Max", &m_AOMax, 0.0f, 1.0f);

		ImGui::Columns(1, "###col");
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::Columns(2, "###col");

		ImGuiUtils::ColColorEdit3("Albedo", &m_Albedo);
		ImGuiUtils::ColSliderFloat("Roughness", &m_Roughness, 0.0f, 1.0f);
		ImGuiUtils::ColSliderFloat("Translucent", &m_Translucent, 0.0f, 3.0f);

		ImGui::Columns(1, "###col");
	}

	ImGui::End();
}

void GrassRenderer::Render(const glm::mat4& mvp, const Camera& cam, const MapGenerator& map,
	const MaterialGenerator& material, const SkyRenderer& sky, const Clipmap& clipmap)
{
	if (!m_RenderGrass) return;

	{
		ProfilerGPUEvent we("Grass::Prepare");

		m_PresentShader->Bind();
		m_PresentShader->setUniform1f("uL", map.getScaleSettings().ScaleXZ);
		m_PresentShader->setUniform3f("uPos", cam.getPos());
		m_PresentShader->setUniformMatrix4fv("uMVP", mvp);

		m_PresentShader->setUniform3f("uLightDir", sky.getSunDir());
		m_PresentShader->setUniform3f("uSunCol", m_SunCol);
		m_PresentShader->setUniform1f("uSunStr", m_SunStr);
		m_PresentShader->setUniform1f("uSkyDiff", m_SkyDiff);
		m_PresentShader->setUniform1f("uSkySpec", m_SkySpec);
		m_PresentShader->setUniform1f("uRefStr", m_RefStr);
		m_PresentShader->setUniform1i("uShadow", int(m_Shadows));

		m_PresentShader->setUniform1f("uGrassHeight", m_GrassHeight);
		m_PresentShader->setUniform1f("uTilingFactor", m_Tiling);
		m_PresentShader->setUniform1f("uMaxDepth", m_MaxDepth);
		m_PresentShader->setUniform1f("uTime", m_Time);
		m_PresentShader->setUniform2f("uScrollVel", m_ScrollingVelocity.x, m_ScrollingVelocity.y);
		m_PresentShader->setUniform1f("uNoiseTiling", m_NoiseTiling);
		m_PresentShader->setUniform1f("uStrength", m_NoiseStrength);
		m_PresentShader->setUniform1f("uSway", m_Sway);
		m_PresentShader->setUniform1f("uAOMin", m_AOMin);
		m_PresentShader->setUniform1f("uAOMax", m_AOMax);
		
		m_PresentShader->setUniform3f("uAlbedo", m_Albedo);
		m_PresentShader->setUniform1f("uRoughness", m_Roughness);
		m_PresentShader->setUniform1f("uTranslucent", m_Translucent);

		m_RaycastResult->Bind(0);
		m_PresentShader->setUniform1i("raycast_res", 0);
		m_Noise->Bind(1);
		m_PresentShader->setUniform1i("noise", 1);


		map.BindNormalmap(2);
		m_PresentShader->setUniform1i("normalmap", 2);
		map.BindShadowmap(3);
		m_PresentShader->setUniform1i("shadowmap", 3);

		sky.BindIrradiance(4);
		m_PresentShader->setUniform1i("irradiance", 4);
		sky.BindPrefiltered(5);
		m_PresentShader->setUniform1i("prefiltered", 5);
	}

	{
		ProfilerGPUEvent we("Grass::Draw");

		auto scale_y = map.getScaleSettings().ScaleY;
		clipmap.BindAndDraw(cam, scale_y, m_LodLevels);
	}
}