#pragma once

#include "Shader.h"
#include "Texture.h"

#include <memory>

class ResourceManager {
public:
	ResourceManager();

	std::shared_ptr<VertFragShader> RequestVertFragShader(const std::string& v_path, const std::string& f_path);
	std::shared_ptr<ComputeShader>  RequestComputeShader(const std::string& path);

	std::shared_ptr<Texture2D>    RequestTexture2D();
	std::shared_ptr<Texture3D>    RequestTexture3D();
	std::shared_ptr<TextureArray> RequestTextureArray();
	std::shared_ptr<Cubemap>      RequestCubemap();

	void ReloadShaders();
	void DrawTextureBrowser(bool& open);
	void OnUpdate();

	template <typename T>
	void RequestPreviewUpdate(std::shared_ptr<T> ptr)
	{
		static_assert(std::is_base_of<Texture, T>::value, 
			"Template argument does not derive from Texture"
		);

		if (std::dynamic_pointer_cast<Texture>(ptr) == m_PreviewPtr)
		{
			m_UpdatePreview = true;
		}
	}

private:
	void UpdatePreview();

	std::vector<std::shared_ptr<Shader>> m_ShaderCache;

	std::vector<std::shared_ptr<Texture2D>>    m_Texture2DCache;
	std::vector<std::shared_ptr<TextureArray>> m_TextureArrayCache;
	std::vector<std::shared_ptr<Cubemap>>      m_CubemapCache;
	std::vector<std::shared_ptr<Texture3D>>    m_Texture3DCache;

	bool m_ReloadShaders = false, m_UpdatePreview = false;

	enum class PreviewType {
		Texture2D, TextureArray, Cubemap, Texture3D
	};

	PreviewType m_PrevType = PreviewType::Texture2D;

	std::shared_ptr<Texture> m_PreviewPtr;

	int m_PreviewLayer = 0, m_PreviewSide = 0, m_PreviewSlice = 0;
	float m_PreviewDepth = 0.0f;

	bool m_PreviewChannels[4] = { true, true, true, true };

	glm::vec2 m_PreviewRange = glm::vec2(0.0f, 1.0f);

	ComputeShader m_Tex2DPrevShader, m_CubePrevShader, m_3DPrevShader;
	Texture2D m_PreviewTexture;
};