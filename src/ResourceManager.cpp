#include "ResourceManager.h"

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

//This is needed to make sure shaders are reloaded between rendering cycles
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
}