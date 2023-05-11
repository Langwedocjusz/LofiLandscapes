#pragma once

#include "Shader.h"

#include <memory>

class ResourceManager {
public:
	std::shared_ptr<VertFragShader> RequestVertFragShader(const std::string& v_path, const std::string& f_path);
	std::shared_ptr<ComputeShader> RequestComputeShader(const std::string& path);

	void ReloadShaders();

	void OnUpdate();

private:
	std::vector<std::shared_ptr<Shader>> m_ShaderCache;

	bool m_ReloadShaders = false;
};