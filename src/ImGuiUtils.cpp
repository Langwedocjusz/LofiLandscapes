#include "ImGuiUtils.h"

#include "imgui.h"

#include "glm/gtc/type_ptr.hpp"

void ImGuiUtils::Checkbox(const std::string& label, bool* value) {
	ImGui::Text(label.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() / 3);

	ImGui::Checkbox(("##" + label).c_str(), value);
}

void ImGuiUtils::SliderInt(const std::string& label, int* value, int min, int max) {
	ImGuiStyle& style = ImGui::GetStyle();
	float padding = style.FramePadding.x;

	ImGui::Text(label.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() / 3);

	ImGui::PushItemWidth(-padding);
	ImGui::SliderInt(("##" + label).c_str(), value, min, max);
	ImGui::PopItemWidth();
}

void ImGuiUtils::SliderFloat(const std::string& label, float* value, float min, float max) {
	ImGuiStyle& style = ImGui::GetStyle();
	float padding = style.FramePadding.x;

	ImGui::Text(label.c_str());
	
	ImGui::SameLine(ImGui::GetWindowWidth() / 3);

	ImGui::PushItemWidth(-padding);
	ImGui::SliderFloat(("##" + label).c_str(), value, min, max);
	ImGui::PopItemWidth();
}

void ImGuiUtils::ColorEdit3(const std::string& label, float* value) {
	ImGuiStyle& style = ImGui::GetStyle();
	float padding = style.FramePadding.x;

	ImGui::Text(label.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() / 3);

	ImGui::PushItemWidth(-padding);
	ImGui::ColorEdit3(("##" + label).c_str(), value);
	ImGui::PopItemWidth();
}

void ImGuiUtils::ColorEdit3(const std::string& label, glm::vec3* value) {
	ImGuiStyle& style = ImGui::GetStyle();
	float padding = style.FramePadding.x;

	ImGui::Text(label.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() / 3);

	ImGui::PushItemWidth(-padding);
	ImGui::ColorEdit3(("##" + label).c_str(), glm::value_ptr(*value));
	ImGui::PopItemWidth();
}

bool ImGuiUtils::Button(const std::string& label) {
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label.c_str()).x + 2.0f * style.FramePadding.x;
	float available = ImGui::GetContentRegionAvail().x;

	float offset = 0.5f * (available - size);

	if (offset > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

	return ImGui::Button(label.c_str());
}