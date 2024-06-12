#include "ResourceManager.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

ResourceManager::ResourceManager()
	: m_Tex2DPrevShader("res/shaders/debug/texture2d_preview.glsl")
	, m_CubePrevShader("res/shaders/debug/cubemap_preview.glsl")
	, m_3DPrevShader("res/shaders/debug/texture3d_preview.glsl")
	, m_PreviewTexture("Preview")
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

std::shared_ptr<Texture2D> ResourceManager::RequestTexture2D(const std::string& name)
{
	m_Texture2DCache.push_back(std::make_shared<Texture2D>(name));
	return m_Texture2DCache.back();
}

std::shared_ptr<Texture3D> ResourceManager::RequestTexture3D(const std::string& name)
{
	m_Texture3DCache.push_back(std::make_shared<Texture3D>(name));
	return m_Texture3DCache.back();
}

std::shared_ptr<TextureArray> ResourceManager::RequestTextureArray(const std::string& name)
{
	m_TextureArrayCache.push_back(std::make_shared<TextureArray>(name));
	return m_TextureArrayCache.back();
}

std::shared_ptr<Cubemap> ResourceManager::RequestCubemap(const std::string& name)
{
	m_CubemapCache.push_back(std::make_shared<Cubemap>(name));
	return m_CubemapCache.back();
}

void ResourceManager::DrawTextureBrowser(bool& open)
{
	ImGui::SetNextWindowSize(ImVec2(500.0f, 500.0f), ImGuiCond_FirstUseEver);

	ImGui::Begin("Texture Browser", &open);

	ImGui::BeginChild("#Texture browser settings", ImVec2(0, 250), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);

	ImGuiUtils::BeginGroupPanel("Currently previewing");
	ImGui::Columns(2, "###col");

	std::vector<std::string> options{ "Texture 2D", "Texture Array", "Cubemap", "Texture 3D" };
	size_t tmp_id = static_cast<int>(m_PrevType);

	ImGuiUtils::ColCombo("Texture type", options, tmp_id);

	m_PrevType = static_cast<PreviewType>(tmp_id);

	std::shared_ptr<Texture> tmp_ptr = m_PreviewPtr;

	static size_t tex_id = 0, tex_arr_id = 0, cube_id = 0, tex3d_id = 0;

	int arr_layer = m_PreviewLayer;
	size_t cube_side = m_PreviewSide;
	float depth_3d = m_PreviewDepth;

	auto ColComboTexture = [](auto& texture_cache, size_t& selected_id)
	{
		ImGui::TextUnformatted("Texture name");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		ImGuiStyle& style = ImGui::GetStyle();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_ButtonHovered]);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.Colors[ImGuiCol_ButtonActive]);

		if (ImGui::BeginCombo("##Texture name", texture_cache.at(selected_id)->getName().c_str()))
		{
			for (size_t n = 0; n < texture_cache.size(); n++)
			{
				bool is_selected = (selected_id == n);

				if (ImGui::Selectable(texture_cache.at(n)->getName().c_str(), is_selected))
					selected_id = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::PopStyleColor(3);

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	};

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			ColComboTexture(m_Texture2DCache, tex_id);

			tmp_ptr = m_Texture2DCache.at(tex_id);
			break;
		}
		case PreviewType::TextureArray:
		{
			ColComboTexture(m_TextureArrayCache, tex_arr_id);

			const int max_layer = std::max(0, static_cast<int>(m_TextureArrayCache.at(tex_arr_id)->getLayers()) - 1);

			if (arr_layer > max_layer)
				arr_layer = max_layer;

			ImGuiUtils::ColSliderInt("Texture layer", &arr_layer, 0, max_layer);

			tmp_ptr = m_TextureArrayCache.at(tex_arr_id);
			break;
		}
		case PreviewType::Cubemap:
		{
			ColComboTexture(m_CubemapCache, cube_id);

			std::vector<std::string> side_names{
				"Positive X", "Negative X",
				"Positive Y", "Negative Y",
				"Positive Z", "Negative Z"
			};

			ImGuiUtils::ColCombo("Selected side", side_names, cube_side);

			tmp_ptr = m_CubemapCache.at(cube_id);
			break;
		}
		case PreviewType::Texture3D:
		{
			ColComboTexture(m_Texture3DCache, tex3d_id);

			ImGuiUtils::ColSliderFloat("Depth", &depth_3d, 0.0, 1.0);

			tmp_ptr = m_Texture3DCache.at(tex3d_id);
			break;
		}
	}

	ImGui::Columns(1, "###col");
	ImGuiUtils::EndGroupPanel();

	ImGuiUtils::BeginGroupPanel("Preview settings");
	ImGui::Columns(2, "###col");

	glm::vec2 preview_range = m_PreviewRange;
	bool channel_flags[4];

	for (int i = 0; i < 4; i++)
		channel_flags[i] = m_PreviewChannels[i];

	ImGuiUtils::ColInputFloat("Scale", &m_PreviewScale, 0.1f, 0.1f);
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
	ImGuiUtils::EndGroupPanel();

	ImGui::EndChild();

	ImGui::BeginChild("#Texture browser display", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);

	const auto avail_region = ImGui::GetContentRegionAvail();

	const float aspect = static_cast<float>(m_PreviewTexture.getResolutionY())
		               / static_cast<float>(m_PreviewTexture.getResolutionX());

	const float size_x = m_PreviewScale * avail_region.x;
	const float size_y = aspect * size_x;

	m_PreviewTexture.DrawToImGui(size_x, size_y);

	ImGui::EndChild();

	ImGui::End();

	bool state_changed = false;

	auto CheckIfChanged = [&state_changed](auto& prev_val, auto& curr_val)
	{
		if (curr_val != prev_val)
		{
			prev_val = curr_val;
			state_changed = true;
		}
	};

	CheckIfChanged(m_PreviewPtr, tmp_ptr);
	CheckIfChanged(m_PreviewLayer, arr_layer);
	CheckIfChanged(m_PreviewSide, cube_side);
	CheckIfChanged(m_PreviewDepth, depth_3d);
	CheckIfChanged(m_PreviewRange, preview_range);

	for (int i=0; i<4; i++)
		CheckIfChanged(m_PreviewChannels[i], channel_flags[i]);

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
	const glm::vec4 channel_flags{
		float(m_PreviewChannels[0]), float(m_PreviewChannels[1]),
		float(m_PreviewChannels[2]), float(m_PreviewChannels[3])
	};

	switch (m_PrevType)
	{
		case PreviewType::Texture2D:
		{
			m_Tex2DPrevShader.Bind();
			m_Tex2DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_Tex2DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			const auto tex_ptr = std::dynamic_pointer_cast<Texture2D>(m_PreviewPtr);

			const uint32_t res_x = tex_ptr->getResolutionX();
			const uint32_t res_y = tex_ptr->getResolutionY();

			m_PreviewTexture.Resize(res_x, res_y);
			m_PreviewTexture.BindImage(0, 0);

			tex_ptr->Bind();

			m_Tex2DPrevShader.Dispatch(res_x, res_y, 1);
			break;
		}
		case PreviewType::TextureArray:
		{
			m_Tex2DPrevShader.Bind();
			m_Tex2DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_Tex2DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			const auto tex_ptr = std::dynamic_pointer_cast<TextureArray>(m_PreviewPtr);

			const uint32_t res_x = tex_ptr->getResolutionX();
			const uint32_t res_y = tex_ptr->getResolutionY();

			m_PreviewTexture.Resize(res_x, res_y);
			m_PreviewTexture.BindImage(0, 0);

			tex_ptr->BindLayer(0, m_PreviewLayer);

			m_Tex2DPrevShader.Dispatch(res_x, res_y, 1);
			break;
		}
		case PreviewType::Cubemap:
		{
			m_CubePrevShader.Bind();
			m_CubePrevShader.setUniform1i("uSide", m_PreviewSide);
			m_CubePrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_CubePrevShader.setUniform4f("uChannelFlags", channel_flags);

			const auto tex_ptr = std::dynamic_pointer_cast<Cubemap>(m_PreviewPtr);

			const uint32_t res = tex_ptr->getResolution();

			m_PreviewTexture.Resize(res, res);
			m_PreviewTexture.BindImage(0, 0);

			tex_ptr->Bind();

			m_CubePrevShader.Dispatch(res, res, 1);
			break;
		}
		case PreviewType::Texture3D:
		{
			m_3DPrevShader.Bind();
			m_3DPrevShader.setUniform1f("uDepth", m_PreviewDepth);
			m_3DPrevShader.setUniform2f("uRange", m_PreviewRange.x, m_PreviewRange.y);
			m_3DPrevShader.setUniform4f("uChannelFlags", channel_flags);

			const auto tex_ptr = std::dynamic_pointer_cast<Texture3D>(m_PreviewPtr);

			const uint32_t res_x = tex_ptr->getResolutionX();
			const uint32_t res_y = tex_ptr->getResolutionY();

			m_PreviewTexture.Resize(res_x, res_y);
			m_PreviewTexture.BindImage(0, 0);

			tex_ptr->Bind();

			m_Tex2DPrevShader.Dispatch(res_x, res_y, 1);
			break;
		}
	}

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}