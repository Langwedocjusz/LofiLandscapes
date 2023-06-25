#include "ResourceManager.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

ResourceManager::ResourceManager()
	: m_Tex2DPrevShader("res/shaders/debug/texture2d_preview.glsl")
	, m_CubePrevShader("res/shaders/debug/cubemap_preview.glsl")
	, m_3DPrevShader("res/shaders/debug/texture3d_preview.glsl")
{
	m_PreviewTexture.Initialize(Texture2DSpec{
		1024, 1024, GL_RGBA8, GL_RGBA,
		GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR,
		GL_CLAMP_TO_EDGE,
		{0.0f, 0.0f, 0.0f, 0.0f}
	});
}

std::shared_ptr<VertFragShader> ResourceManager::RequestVertFragShader(const std::string& v_path, const std::string& f_path)
{
	m_ShaderCache.push_back(std::make_shared<VertFragShader>(v_path, f_path));
	return std::dynamic_pointer_cast<VertFragShader>(m_ShaderCache.back());
}

std::shared_ptr<ComputeShader> ResourceManager::RequestComputeShader(const std::string& path)
{
	m_ShaderCache.push_back(std::make_shared<ComputeShader>(path));
	return std::dynamic_pointer_cast<ComputeShader>(m_ShaderCache.back());
}

void ResourceManager::ReloadShaders()
{
	m_ReloadShaders = true;
}

std::shared_ptr<Texture2D> ResourceManager::RequestTexture2D()
{
	m_Texture2DCache.push_back(std::make_shared<Texture2D>());
	return m_Texture2DCache.back();
}

std::shared_ptr<Texture3D> ResourceManager::RequestTexture3D()
{
	m_Texture3DCache.push_back(std::make_shared<Texture3D>());
	return m_Texture3DCache.back();
}

std::shared_ptr<TextureArray> ResourceManager::RequestTextureArray()
{
	m_TextureArrayCache.push_back(std::make_shared<TextureArray>());
	return m_TextureArrayCache.back();
}

std::shared_ptr<Cubemap> ResourceManager::RequestCubemap()
{
	m_CubemapCache.push_back(std::make_shared<Cubemap>());
	return m_CubemapCache.back();
}

void ResourceManager::DrawTextureBrowser(bool& open)
{
	ImGui::Begin("Texture Browser", &open);

	std::shared_ptr<Texture> tmp_ptr;

	static int tex_id = 0, tex_arr_id = 0, cube_id = 0, tex3d_id = 0;
	static int arr_layer = 0, cube_side = 0, slice_3d = 0;
	static float depth_3d = 0.0f;

	std::vector<std::string> options{ "Texture", "Texture Array", "Cubemap", "Texture 3D" };
	int tmp_id = static_cast<int>(m_PrevType);

	ImGui::Columns(2, "###col");
	ImGuiUtils::Combo("Currently previewing", options, tmp_id);

	m_PrevType = static_cast<PreviewType>(tmp_id);

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			int max_id = std::max(0, int(m_Texture2DCache.size()) - 1);

			ImGuiUtils::SliderInt("Texture ID", &tex_id, 0, max_id);

			tmp_ptr = m_Texture2DCache.at(tex_id);
			break;
		}
		case PreviewType::TextureArray:
		{
			int max_id = std::max(0, int(m_TextureArrayCache.size()) - 1);

			ImGuiUtils::SliderInt("Texture ID", &tex_arr_id, 0, max_id);
			ImGuiUtils::SliderInt("Texture layer", &arr_layer, 0, 8);

			tmp_ptr = m_TextureArrayCache.at(tex_arr_id);
			break;
		}
		case PreviewType::Cubemap:
		{
			int max_id = std::max(0, int(m_CubemapCache.size()) - 1);

			ImGuiUtils::SliderInt("Texture ID", &cube_id, 0, max_id);
			
			std::vector<std::string> side_names{ 
				"Positive X", "Negative X",
				"Positive Y", "Negative Y",
				"Positive Z", "Negative Z"
			};

			ImGuiUtils::Combo("Selected side", side_names, cube_side);

			tmp_ptr = m_CubemapCache.at(cube_id);
			break;
		}
		case PreviewType::Texture3D:
		{
			int max_id = std::max(0, int(m_Texture3DCache.size()) - 1);

			ImGuiUtils::SliderInt("Texture ID", &tex3d_id, 0, max_id);
			//ImGuiUtils::SliderInt("Slice", &slice_3d, 0, 32);
			ImGuiUtils::SliderFloat("Depth", &depth_3d, 0.0, 1.0);

			tmp_ptr = m_Texture3DCache.at(tex3d_id);
			break;
		}
	}

	bool state_changed = (tmp_ptr != m_PreviewPtr);

	state_changed |= (m_PreviewLayer != arr_layer);
	state_changed |= (m_PreviewSide != cube_side);
	//state_changed |= (m_PreviewSlice != slice_3d);
	state_changed |= (m_PreviewDepth != depth_3d);

	if (state_changed)
	{
		m_PreviewPtr = tmp_ptr;
		m_PreviewLayer = arr_layer;
		m_PreviewSide = cube_side;
		//m_PreviewSlice = slice_3d;
		m_PreviewDepth = depth_3d;

		m_UpdatePreview = true;
	}

	ImGui::Columns(1, "###col");

	ImGui::BeginChild("#Texture browser display", ImVec2(0.0f, 0.0f), true);

	const auto avail_region = ImGui::GetContentRegionAvail();
	const float size = std::min(avail_region.x, avail_region.y);
	
	m_PreviewTexture.DrawToImGui(size, size);

	ImGui::EndChild();

	ImGui::End();
}

void ResourceManager::OnUpdate()
{
	if (m_ReloadShaders)
	{
		for (auto& shader : m_ShaderCache)
		{
			shader->Reload();
		}

		m_ReloadShaders = false;
	}

	if (m_UpdatePreview)
	{
		UpdatePreview();
		m_UpdatePreview = false;
	}
}

void ResourceManager::UpdatePreview()
{
	m_PreviewTexture.BindImage(0, 0);

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			m_Tex2DPrevShader.Bind();
			std::dynamic_pointer_cast<Texture2D>(m_PreviewPtr)->Bind();
			break;
		}
		case PreviewType::TextureArray:
		{
			m_Tex2DPrevShader.Bind();
			std::dynamic_pointer_cast<TextureArray>(m_PreviewPtr)->BindLayer(0, m_PreviewLayer);
			break;
		}
		case PreviewType::Cubemap:
		{
			m_CubePrevShader.Bind();
			m_CubePrevShader.setUniform1i("uSide", m_PreviewSide);

			std::dynamic_pointer_cast<Cubemap>(m_PreviewPtr)->Bind();
			break;
		}
		case PreviewType::Texture3D:
		{
			m_3DPrevShader.Bind();
			m_3DPrevShader.setUniform1f("uDepth", m_PreviewDepth);

			std::dynamic_pointer_cast<Texture3D>(m_PreviewPtr)->Bind();
			break;
		}
	}

	//Assumes square texture
	const auto res = m_PreviewTexture.getSpec().ResolutionX;

	glDispatchCompute(res / 32, res / 32, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}