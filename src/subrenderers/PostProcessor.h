#pragma once

#include "ResourceManager.h"

class PostProcessor {
public:
	PostProcessor(ResourceManager& manager, const FramebufferTexture& framebuffer);

	void Init(uint32_t width, uint32_t height);
	void OnRender();
	void OnImGui(bool open);
	
	void ResizeBuffers(uint32_t width, uint32_t height);

	void BindOutput(int id);

private:
	bool m_OutputIsFront = true;
	bool m_FirstPass = true;

	void BindInputTexture(int id);
	void BindOutputImage(int id, int mip);
	void SwapBuffers();

	std::shared_ptr<Texture2D> m_FrontBuffer;
	std::shared_ptr<Texture2D> m_BackBuffer;

	void DoFXAA();
	bool m_EnableFXAA = true;
	std::shared_ptr<ComputeShader> m_LuminanceShader;
	std::shared_ptr<ComputeShader> m_ContrastShader;
	float m_FXAAContrastThreshold = 0.0625f;
	float m_FXAARelativeThreshold = 0.166f;
	float m_FXAASubpixelAmount = 0.75f;

	ResourceManager& m_ResourceManager;
	const FramebufferTexture& m_Framebuffer;
};