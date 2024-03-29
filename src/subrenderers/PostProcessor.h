#pragma once

#include "ResourceManager.h"
#include "Framebuffer.h"

class PostProcessor {
public:
	PostProcessor(ResourceManager& manager, const Framebuffer& framebuffer);

	void Init(int width, int height);
	void OnRender();
	void OnImGui(bool& open);
	
	void ResizeBuffers(int width, int height);

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
	const Framebuffer& m_Framebuffer;
};