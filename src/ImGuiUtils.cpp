#include "ImGuiUtils.h"

#include "imgui.h"

#include "glm/gtc/type_ptr.hpp"

void ImGuiUtils::Checkbox(const std::string& label, bool* value, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::Checkbox(("##" + label + suffix).c_str(), value);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::SliderInt(const std::string& label, int* value, int min, int max, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::SliderInt(("##" + label + suffix).c_str(), value, min, max);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::SliderIntLog(const std::string& label, int* value, int min, int max, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::SliderInt(("##" + label + suffix).c_str(), value, min, max, "%d", ImGuiSliderFlags_Logarithmic);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::SliderFloat(const std::string& label, float* value, float min, float max, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::SliderFloat(("##" + label + suffix).c_str(), value, min, max);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::ColorEdit3(const std::string& label, float* value, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::ColorEdit3(("##" + label + suffix).c_str(), value);
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::ColorEdit3(const std::string& label, glm::vec3* value, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
	ImGui::ColorEdit3(("##" + label + suffix).c_str(), glm::value_ptr(*value));
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::Combo(const std::string& label, const std::vector<std::string> options, 
	                   int& selected_id, const std::string& suffix) {
	ImGui::Text(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);

	if (ImGui::BeginCombo(("##" + label + suffix).c_str(), options[selected_id].c_str()))
	{
		for (int n = 0; n < options.size(); n++)
		{
			bool is_selected = (selected_id == n);

			if (ImGui::Selectable(options[n].c_str(), is_selected)) {
				selected_id = n;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::PopItemWidth();
	ImGui::NextColumn();
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

void ImGuiUtils::Separator() {
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Separator();
}