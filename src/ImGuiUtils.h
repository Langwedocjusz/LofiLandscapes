#pragma once

#include <string>

#include "glm/glm.hpp"

namespace ImGuiUtils {
	void Checkbox(const std::string& label, bool* value);
	void SliderInt(const std::string& label, int* value, int min, int max);
	void SliderFloat(const std::string& label, float* value, float min, float max);
	void ColorEdit3(const std::string& label, float* value);
	void ColorEdit3(const std::string& label, glm::vec3* value);
	bool Button(const std::string& label);
}