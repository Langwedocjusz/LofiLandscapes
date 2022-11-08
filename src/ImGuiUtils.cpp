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