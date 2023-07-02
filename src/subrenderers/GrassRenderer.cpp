#include "GrassRenderer.h"

#include "Profiler.h"

#include "glad/glad.h"
#include "imgui.h"

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
		64, 64, 64,
		GL_RGBA16F, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
		GL_REPEAT,
		{0.0f, 0.0f, 0.0f, 0.0f}
	});

	m_Noise->Initialize(Texture2DSpec{
		256, 256,
		GL_RGBA16F, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
		GL_REPEAT,
		{0.0f, 0.0f, 0.0f, 0.0f}
	});

	OnUpdate(0.0f);
}

void GrassRenderer::OnUpdate(float deltatime)
{
	if (m_UpdateRaycast)
	{
		ProfilerGPUEvent we("Grass::Raycast");

		auto res_x = m_RaycastResult->getSpec().ResolutionX;
		auto res_y = m_RaycastResult->getSpec().ResolutionY;
		auto res_z = m_RaycastResult->getSpec().ResolutionZ;

		m_RaycastResult->BindImage(0, 0);

		m_RaycastShader->Bind();

		glDispatchCompute(res_x / 32, res_y / 32, res_z);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		m_ResourceManager.RequestPreviewUpdate(m_RaycastResult);

		m_UpdateRaycast = false;

		//Temporary: also update noise here
		res_x = m_Noise->getSpec().ResolutionX;
		res_y = m_Noise->getSpec().ResolutionY;
		
		m_Noise->BindImage(0, 0);

		m_NoiseGenerator->Bind();
		m_NoiseGenerator->setUniform1f("uScale", m_NoiseScale);
		m_NoiseGenerator->setUniform1f("uStrength", m_NoiseStrength);

		glDispatchCompute(res_x / 32, res_y / 32, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		m_ResourceManager.RequestPreviewUpdate(m_Noise);
	}

	m_Time += deltatime;
	if (m_Time > 1e3) m_Time = 0.0f;
}

void GrassRenderer::OnImGui(bool& open)
{
	ImGui::Begin("Infinite noodles", &open);

	if (ImGui::CollapsingHeader("Precomputed raycast"))
	{
		if (ImGui::Button("Redraw"))
			m_UpdateRaycast = true;
	}
	
	if (ImGui::CollapsingHeader("Rendering"))
	{
		ImGui::Checkbox("Render noodles", &m_RenderGrass);

		ImGui::SliderInt("Lod Levels", &m_LodLevels, 0, 5);

		ImGui::SliderFloat("Height", &m_GrassHeight, 0.0f, 10.0f);
		ImGui::SliderFloat("Tiling", &m_Tiling, 0.0f, 10.0f);
		ImGui::SliderFloat("MaxDepth", &m_MaxDepth, 0.0f, 10.0f);

		//ImGui::DragFloat3("Slant", glm::value_ptr(m_Slant), -1.0f, 1.0f);
		ImGui::SliderFloat("NoiseTiling", &m_NoiseTiling, 0.0f, 10.0f);
		ImGui::SliderFloat("NoiseScale", &m_NoiseScale, 0.0f, 10.0f);
		ImGui::SliderFloat("NoiseStrength", &m_NoiseStrength, 0.0f, 10.0f);

		ImGui::DragFloat2("Velocity", glm::value_ptr(m_ScrollingVelocity));
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
		//m_PresentShader->setUniform3f("uSlant", m_Slant);
		m_PresentShader->setUniform1f("uTime", m_Time);
		m_PresentShader->setUniform2f("uScrollVel", m_ScrollingVelocity.x, m_ScrollingVelocity.y);
		m_PresentShader->setUniform1f("uNoiseTiling", m_NoiseTiling);

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