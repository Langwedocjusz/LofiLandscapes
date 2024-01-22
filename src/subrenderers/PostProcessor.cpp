#include "PostProcessor.h"

#include "ImGuiUtils.h"
#include "ImGuiIcons.h"

#include "Profiler.h"

#include "glad/glad.h"

PostProcessor::PostProcessor(ResourceManager& manager, const Framebuffer& framebuffer)
	: m_ResourceManager(manager)
	, m_Framebuffer(framebuffer)
{
	m_LuminanceShader = m_ResourceManager.RequestComputeShader("res/shaders/post/calc_luminance.glsl");
	m_ContrastShader = m_ResourceManager.RequestComputeShader("res/shaders/post/fxaa.glsl");

	m_FrontBuffer = m_ResourceManager.RequestTexture2D();
	m_BackBuffer = m_ResourceManager.RequestTexture2D();
}

void PostProcessor::Init(uint32_t width, uint32_t height)
{
	Texture2DSpec buffer_spec{
		width, height, GL_RGBA8, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
		GL_MIRRORED_REPEAT,
		{0.0f, 0.0f, 0.0f, 0.0f}
	};

	m_FrontBuffer->Initialize(buffer_spec);
	m_BackBuffer->Initialize(buffer_spec);
}

void PostProcessor::OnRender()
{
	m_FirstPass = true;
	m_OutputIsFront = true;

	if (m_EnableFXAA) DoFXAA();

	m_ResourceManager.RequestPreviewUpdate(m_FrontBuffer);
	m_ResourceManager.RequestPreviewUpdate(m_BackBuffer);
}

void PostProcessor::DoFXAA()
{
	ProfilerGPUEvent we("FXAA");

	{
		BindOutputImage(0, 0);
		BindInputTexture(0);

		m_LuminanceShader->Bind();
		m_LuminanceShader->setUniform1i("InputTexture", 0);

		const auto res_x = m_FrontBuffer->getResolutionX();
		const auto res_y = m_FrontBuffer->getResolutionY();

		m_LuminanceShader->Dispatch(res_x, res_y, 1);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		SwapBuffers();
	}

	{
		BindOutputImage(0, 0);
		BindInputTexture(0);

		m_ContrastShader->Bind();
		m_ContrastShader->setUniform1i("InputTexture", 0);
		m_ContrastShader->setUniform1f("uContrastThreshold", m_FXAAContrastThreshold);
		m_ContrastShader->setUniform1f("uRelativeThreshold", m_FXAARelativeThreshold);
		m_ContrastShader->setUniform1f("uSubpixelAmount", m_FXAASubpixelAmount);

		const auto res_x = m_BackBuffer->getResolutionX();
		const auto res_y = m_BackBuffer->getResolutionY();

		m_ContrastShader->Dispatch(res_x, res_y, 1);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		SwapBuffers();
	}
}

void PostProcessor::OnImGui(bool& open)
{
	ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f), ImGuiCond_FirstUseEver);

	ImGui::Begin(LOFI_ICONS_POSTFX "Postprocessing", &open, ImGuiWindowFlags_NoFocusOnAppearing);

	if (ImGui::CollapsingHeader("FXAA"))
	{
		ImGui::Columns(2, "###col");
		ImGuiUtils::ColCheckbox("Enable FXAA", &m_EnableFXAA);
		ImGuiUtils::ColSliderFloat("Contrast Threshold", &m_FXAAContrastThreshold, 0.0312f, 0.0833f);
		ImGuiUtils::ColSliderFloat("Relative Threshold", &m_FXAARelativeThreshold, 0.063f, 0.333f);
		ImGuiUtils::ColSliderFloat("Subpixel Amount", &m_FXAASubpixelAmount, 0.5f, 1.0f);
		ImGui::Columns(1, "###col");
	}

	ImGui::End();
}

void PostProcessor::ResizeBuffers(uint32_t width, uint32_t height)
{
	m_FrontBuffer->Resize(width, height);
	m_BackBuffer->Resize(width, height);
}

void PostProcessor::BindOutput(int id)
{
	if (m_FirstPass)
		m_Framebuffer.BindColorTex(id);

	else
	{
		//If SwapBuffers() on last pass set Front as output,
		//then Back must contain newest data
		if (m_OutputIsFront)
			m_BackBuffer->Bind(id);
		else
			m_FrontBuffer->Bind(id);
	}
}

void PostProcessor::BindInputTexture(int id)
{
	if (m_FirstPass)
	{
		m_Framebuffer.BindColorTex(id);
		m_FirstPass = false;
	}

	else
	{
		if (m_OutputIsFront)
			m_BackBuffer->Bind(id);
		else
			m_FrontBuffer->Bind(id);
	}
}

void PostProcessor::BindOutputImage(int id, int mip)
{
	if (m_OutputIsFront)
		m_FrontBuffer->BindImage(id, mip);
	else
		m_BackBuffer->BindImage(id, mip);
}

void PostProcessor::SwapBuffers()
{
	m_OutputIsFront = !m_OutputIsFront;
}