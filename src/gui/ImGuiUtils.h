#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

namespace ImGuiUtils {

	//Adds a hollow square to current window draw list, meant to debug positions/sizes of things
	void DebugRect(ImVec2 min, ImVec2 max, ImU32 color = IM_COL32(0, 255, 0, 255));

	//Returns the width of the rectangle of the last item drawn
	float GetLastItemRectWidth();

	//Makes combo use std containers and makes it visually consistent with a button
	void Combo(const std::string& label, const std::vector<std::string>& options, size_t& selected_id);

	//Centered button with size corresponding to label length
	bool ButtonCentered(const std::string& label);

	//Separator with a fixed vertical spacing afterwards
	void Separator();

	//Nice frame auto adjusting to fit its contents, taken from
	//https://github.com/ocornut/imgui/issues/1496
	void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f));
	void EndGroupPanel();

	//Variants of the default functions to be called inside two-column pages
	void ColCheckbox(const std::string& label, bool* value, const std::string& suffix = "");
	void ColSliderInt(const std::string& label, int* value, int min, int max, const std::string& suffix = "");
	void ColSliderIntLog(const std::string& label, int* value, int min, int max, const std::string& suffix = "");
	void ColSliderFloat(const std::string& label, float* value, float min, float max, const std::string& suffix = "");
	void ColInputFloat(const std::string& label, float* value, float step = 0.0f, float step_fast = 0.0f, const std::string& suffix = "");
	void ColDragFloat2(const std::string& label, float* value, float speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const std::string& suffix = "");
	void ColColorEdit3(const std::string& label, float* value, const std::string& suffix = "");
	void ColColorEdit3(const std::string& label, glm::vec3* value, const std::string& suffix = "");
	void ColCombo(const std::string& label, const std::vector<std::string>& options, size_t& selected_id, const std::string& suffix = "");
}