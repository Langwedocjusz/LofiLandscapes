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
	static float preview_scale = 1.0f;
	static glm::vec2 preview_range = glm::vec2(0.0f, 1.0f);
	static bool channel_flags[4]{ true, true, true, true };

	std::vector<std::string> options{ "Texture", "Texture Array", "Cubemap", "Texture 3D" };
	int tmp_id = static_cast<int>(m_PrevType);

	ImGui::Columns(2, "###col");
	ImGuiUtils::ColCombo("Currently previewing", options, tmp_id);

	m_PrevType = static_cast<PreviewType>(tmp_id);

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			int max_id = std::max(0, int(m_Texture2DCache.size()) - 1);

			ImGuiUtils::ColSliderInt("Texture ID", &tex_id, 0, max_id);

			tmp_ptr = m_Texture2DCache.at(tex_id);
			break;
		}
		case PreviewType::TextureArray:
		{
			int max_id = std::max(0, int(m_TextureArrayCache.size()) - 1);

			ImGuiUtils::ColSliderInt("Texture ID", &tex_arr_id, 0, max_id);
			ImGuiUtils::ColSliderInt("Texture layer", &arr_layer, 0, 8);

			tmp_ptr = m_TextureArrayCache.at(tex_arr_id);
			break;
		}
		case PreviewType::Cubemap:
		{
			int max_id = std::max(0, int(m_CubemapCache.size()) - 1);

			ImGuiUtils::ColSliderInt("Texture ID", &cube_id, 0, max_id);

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

			ImGuiUtils::ColSliderInt("Texture ID", &tex3d_id, 0, max_id);
			ImGuiUtils::ColSliderFloat("Depth", &depth_3d, 0.0, 1.0);

			tmp_ptr = m_Texture3DCache.at(tex3d_id);
			break;
		}
	}

	ImGui::Separator();

	ImGuiUtils::ColInputFloat("Scale", &preview_scale, 0.1f, 0.1f);
	ImGuiUtils::ColDragFloat2("Range", glm::value_ptr(preview_range), 0.01f);

	ImGui::Text("Active channels");
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::Checkbox("R", &channel_flags[0]);
	ImGui::SameLine();
	ImGui::Checkbox("G", &channel_flags[1]);
	ImGui::SameLine();
	ImGui::Checkbox("B", &channel_flags[2]);
	ImGui::SameLine();
	ImGui::Checkbox("A", &channel_flags[3]);
	ImGui::PopItemWidth();
	ImGui::NextColumn();

	ImGui::Columns(1, "###col");

	ImGui::BeginChild("#Texture browser display", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	const auto avail_region = ImGui::GetContentRegionAvail();
	const float size = preview_scale * std::min(avail_region.x, avail_region.y);
	
	m_PreviewTexture.DrawToImGui(size, size);

	ImGui::EndChild();

	ImGui::End();

	bool state_changed = false;

	auto CheckIfChanged = [](auto& prev_val, auto& curr_val, bool& state_changed) {
		if (curr_val != prev_val)
		{
			prev_val = curr_val;
			state_changed = true;
		}
	};

	CheckIfChanged(m_PreviewPtr, tmp_ptr, state_changed);
	CheckIfChanged(m_PreviewLayer, arr_layer, state_changed);
	CheckIfChanged(m_PreviewSide, cube_side, state_changed);
	CheckIfChanged(m_PreviewDepth, depth_3d, state_changed);
	CheckIfChanged(m_PreviewRange, preview_range, state_changed);

	for (int i=0; i<4; i++)
		CheckIfChanged(m_PreviewChannels[i], channel_flags[i], state_changed);

	if (state_changed)
	{
		m_UpdatePreview = true;
	}
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

	const glm::vec4 channel_flags{ 
		float(m_PreviewChannels[0]), float(m_PreviewChannels[1]), 
		float(m_PreviewChannels[2]), float(m_PreviewChannels[3])
	};

	//Assumes square texture
	const auto res = m_PreviewTexture.getSpec().ResolutionX;

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			m_Tex2DPrevShader.Bind();
			m_Tex2DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_Tex2DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			std::dynamic_pointer_cast<Texture2D>(m_PreviewPtr)->Bind();

			m_Tex2DPrevShader.Dispatch(res, res, 1);
			break;
		}
		case PreviewType::TextureArray:
		{
			m_Tex2DPrevShader.Bind();
			m_Tex2DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_Tex2DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			std::dynamic_pointer_cast<TextureArray>(m_PreviewPtr)->BindLayer(0, m_PreviewLayer);

			m_Tex2DPrevShader.Dispatch(res, res, 1);
			break;
		}
		case PreviewType::Cubemap:
		{
			m_CubePrevShader.Bind();
			m_CubePrevShader.setUniform1i("uSide", m_PreviewSide);
			m_CubePrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_CubePrevShader.setUniform4f("uChannelFlags", channel_flags);

			std::dynamic_pointer_cast<Cubemap>(m_PreviewPtr)->Bind();

			m_CubePrevShader.Dispatch(res, res, 1);
			break;
		}
		case PreviewType::Texture3D:
		{
			m_3DPrevShader.Bind();
			m_3DPrevShader.setUniform1f("uDepth", m_PreviewDepth);
			m_3DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_3DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			std::dynamic_pointer_cast<Texture3D>(m_PreviewPtr)->Bind();

			m_3DPrevShader.Dispatch(res, res, 1);
			break;
		}
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}