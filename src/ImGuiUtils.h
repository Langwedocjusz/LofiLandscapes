#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace ImGuiUtils {
	void Checkbox(const std::string& label, bool* value, const std::string& suffix = "");
	void SliderInt(const std::string& label, int* value, int min, int max, const std::string& suffix = "");
	void SliderIntLog(const std::string& label, int* value, int min, int max, const std::string& suffix = "");
	void SliderFloat(const std::string& label, float* value, float min, float max, const std::string& suffix = "");
	void InputFloat(const std::string& label, float* value, float step = 0.0f, float step_fast = 0.0f, const std::string& suffix = "");
	void DragFloat2(const std::string& label, float* value, float speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const std::string& suffix = "");
	void ColorEdit3(const std::string& label, float* value, const std::string& suffix = "");
	void ColorEdit3(const std::string& label, glm::vec3* value, const std::string& suffix = "");
	void Combo(const std::string& label, const std::vector<std::string> options,
		       int& selected_id, const std::string& suffix = "");

	void Separator();

	bool Button(const std::string& label);
}