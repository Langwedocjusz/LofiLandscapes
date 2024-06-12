#include "ImGuiUtils.h"

#include "glm/gtc/type_ptr.hpp"

#include "imgui_internal.h"

void ImGuiUtils::DebugRect(ImVec2 min, ImVec2 max, ImU32 color)
{
	auto flist = ImGui::GetForegroundDrawList();

	flist->AddRect(min, max, color);
}

float GetLastItemRectWidth()
{
	ImGuiContext& g = *GImGui;

	const ImVec2 rmax = g.LastItemData.Rect.Max;
	const ImVec2 rmin = g.LastItemData.Rect.Min;

	ImGuiUtils::DebugRect(rmin, rmax);

	return std::abs(rmax.x - rmin.x);
}

void ImGuiUtils::Combo(const std::string& label, const std::vector<std::string>& options, size_t& selected_id)
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_ButtonHovered]);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.Colors[ImGuiCol_ButtonActive]);

	if (ImGui::BeginCombo(label.c_str(), options[selected_id].c_str()))
	{
		for (size_t n = 0; n < options.size(); n++)
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

	ImGui::PopStyleColor(3);
}

bool ImGuiUtils::ButtonCentered(const std::string& label) {
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

//========Group panel code, taken from https://github.com/ocornut/imgui/issues/1496

static ImVector<ImRect> s_GroupPanelLabelStack;

void ImGuiUtils::BeginGroupPanel(const char* name, const ImVec2& size)
{
	ImGui::BeginGroup();

	auto itemSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	auto frameHeight = ImGui::GetFrameHeight();
	ImGui::BeginGroup();

	ImVec2 effectiveSize = size;
	if (size.x < 0.0f)
		effectiveSize.x = ImGui::GetContentRegionAvail().x;
	else
		effectiveSize.x = size.x;
	ImGui::Dummy(ImVec2(effectiveSize.x, 0.0f));

	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::BeginGroup();
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::TextUnformatted(name);
	auto labelMin = ImGui::GetItemRectMin();
	auto labelMax = ImGui::GetItemRectMax();
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::Dummy(ImVec2(0.0, frameHeight + itemSpacing.y));
	ImGui::BeginGroup();

	ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
	ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
	ImGui::GetCurrentWindow()->Size.x -= frameHeight;

	auto itemWidth = ImGui::CalcItemWidth();
	ImGui::PushItemWidth(ImMax(0.0f, itemWidth - frameHeight));

	s_GroupPanelLabelStack.push_back(ImRect(labelMin, labelMax));
}

void ImGuiUtils::EndGroupPanel()
{
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	ImGui::PopItemWidth();

	auto itemSpacing = ImGui::GetStyle().ItemSpacing;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	auto frameHeight = ImGui::GetFrameHeight();

	ImGui::EndGroup();

	ImGui::EndGroup();

	ImGui::SameLine(0.0f, 0.0f);
	ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));
	ImGui::Dummy(ImVec2(0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y));

	ImGui::EndGroup();

	auto itemMin = ImGui::GetItemRectMin();
	auto itemMax = ImGui::GetItemRectMax();

	auto labelRect = s_GroupPanelLabelStack.back();
	s_GroupPanelLabelStack.pop_back();

	ImVec2 halfFrame = ImVec2(frameHeight * 0.25f, frameHeight) * 0.5f;
	ImRect frameRect = ImRect(itemMin + halfFrame, itemMax - ImVec2(halfFrame.x, 0.0f));
	labelRect.Min.x -= itemSpacing.x;
	labelRect.Max.x += itemSpacing.x;
	for (int i = 0; i < 4; ++i)
	{
		switch (i)
		{
			// left half-plane
		case 0: ImGui::PushClipRect(ImVec2(-FLT_MAX, -FLT_MAX), ImVec2(labelRect.Min.x, FLT_MAX), true); break;
			// right half-plane
		case 1: ImGui::PushClipRect(ImVec2(labelRect.Max.x, -FLT_MAX), ImVec2(FLT_MAX, FLT_MAX), true); break;
			// top
		case 2: ImGui::PushClipRect(ImVec2(labelRect.Min.x, -FLT_MAX), ImVec2(labelRect.Max.x, labelRect.Min.y), true); break;
			// bottom
		case 3: ImGui::PushClipRect(ImVec2(labelRect.Min.x, labelRect.Max.y), ImVec2(labelRect.Max.x, FLT_MAX), true); break;
		}

		ImGui::GetWindowDrawList()->AddRect(
			frameRect.Min, frameRect.Max,
			ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border)),
			halfFrame.x);

		ImGui::PopClipRect();
	}

	ImGui::PopStyleVar(2);

#if IMGUI_VERSION_NUM >= 17301
	ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
	ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
	ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
	ImGui::GetCurrentWindow()->Size.x += frameHeight;

	ImGui::Dummy(ImVec2(0.0f, 0.0f));

	ImGui::EndGroup();
}

//=====Column versions of basic functions=======================================================

void ColumnPrefix(const std::string& label)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::NextColumn();
	ImGui::PushItemWidth(-1);
}

void ColumnPostfix()
{
	ImGui::PopItemWidth();
	ImGui::NextColumn();
}

void ImGuiUtils::ColCheckbox(const std::string& label, bool* value, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::Checkbox(("##" + label + suffix).c_str(), value);
	ColumnPostfix();
}

void ImGuiUtils::ColSliderInt(const std::string& label, int* value, int min, int max, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::SliderInt(("##" + label + suffix).c_str(), value, min, max);
	ColumnPostfix();
}

void ImGuiUtils::ColSliderIntLog(const std::string& label, int* value, int min, int max, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::SliderInt(("##" + label + suffix).c_str(), value, min, max, "%d", ImGuiSliderFlags_Logarithmic);
	ColumnPostfix();
}

void ImGuiUtils::ColSliderFloat(const std::string& label, float* value, float min, float max, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::SliderFloat(("##" + label + suffix).c_str(), value, min, max);
	ColumnPostfix();
}

void ImGuiUtils::ColInputFloat(const std::string& label, float* value, float step, float step_fast, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::InputFloat(("##" + label + suffix).c_str(), value, step, step_fast);
	ColumnPostfix();
}

void ImGuiUtils::ColDragFloat2(const std::string& label, float* value, float speed, float v_min, float v_max, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::DragFloat2(("##" + label + suffix).c_str(), value, speed, v_min, v_max);
	ColumnPostfix();
}

void ImGuiUtils::ColColorEdit3(const std::string& label, float* value, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::ColorEdit3(("##" + label + suffix).c_str(), value);
	ColumnPostfix();
}

void ImGuiUtils::ColColorEdit3(const std::string& label, glm::vec3* value, const std::string& suffix) {
	ColumnPrefix(label);
	ImGui::ColorEdit3(("##" + label + suffix).c_str(), glm::value_ptr(*value));
	ColumnPostfix();
}

void ImGuiUtils::ColCombo(const std::string& label, const std::vector<std::string>& options, size_t& selected_id, const std::string& suffix)
{
	ColumnPrefix(label);
	Combo(("##" + label + suffix).c_str(), options, selected_id);
	ColumnPostfix();
}