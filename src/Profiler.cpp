#include "Profiler.h"
#include "ImGuiUtils.h"

#include "imgui.h"
#include "glm/glm.hpp"

FrameData::FrameData()
{
	const int start_capacity = 10;

	timings.reserve(start_capacity);
	ids.reserve(start_capacity);
}

//=====Profiler static variables=====================

const int Profiler::s_NumFrames = 48;
int Profiler::s_SelectedFrame = s_NumFrames - 2;
bool Profiler::s_StopProfiling = false;
float Profiler::s_MaxHeight = 17.0f;

std::vector<std::string> Profiler::s_EventLabels;
std::deque<FrameData> Profiler::s_Frames;

//===================================================

size_t Profiler::GetEventID(const std::string& name)
{
	auto idx = std::find(s_EventLabels.begin(), s_EventLabels.end(), name);

	if (idx != s_EventLabels.end())
	{
		return std::distance(s_EventLabels.begin(), idx);
	}

	else
	{
		s_EventLabels.push_back(name);
		return s_EventLabels.size() - 1;
	}
}

void Profiler::SetEventTime(size_t idx, float time)
{
	if (s_StopProfiling) return;

	if (!s_Frames.empty())
	{
		s_Frames.back().timings.push_back(time);
		s_Frames.back().ids.push_back(idx);
	}
}

void Profiler::NextFrame()
{
	if (s_StopProfiling) return;

	if (s_Frames.size() >= s_NumFrames)
	{
		s_Frames.pop_front();
		s_Frames.push_back(FrameData());
	}

	else
	{
		s_Frames.push_back(FrameData());
	}
}

void Profiler::OnImGui(bool& open)
{
	ImGui::Begin("Profiler", &open);

	const float legend_width = 250.0f, min_graph_width = 200.0f;
	const float cpu_timing_width = std::max(min_graph_width, ImGui::GetContentRegionAvail().x - legend_width);
	const float cpu_timing_height = std::max(250.0f, 0.5f * ImGui::GetContentRegionAvail().y);

	ImGui::BeginChild("CPU Timing", ImVec2(cpu_timing_width, cpu_timing_height), true);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	auto ImToGlm = [](ImVec2 v) {return glm::vec2(v.x, v.y); };

	const glm::vec2 start = ImToGlm(ImGui::GetCursorScreenPos());
	const glm::vec2 graph_size = ImToGlm(ImGui::GetContentRegionAvail());
	const float graph_width = graph_size.x, graph_height = graph_size.y;

	float frame_width = graph_width / (s_NumFrames - 1);

	const int num_colors = 6;
	const ImU32 color_palette[num_colors]{
		//https://flatuicolors.com/palette/defo
		IM_COL32(26, 188, 156, 255),
		IM_COL32(46, 204, 113, 255),
		IM_COL32(52, 152, 219, 255),
		IM_COL32(155, 89, 182, 255),
		IM_COL32(52, 73, 94, 255),
		IM_COL32(22, 160, 133, 255)
	};

	for (size_t frame_id = 0; frame_id <s_Frames.size() - 1; frame_id++)
	{
		auto& frame = s_Frames.at(frame_id);

		float height_offset = 0.0f;

		for (size_t event_id = 0; event_id < frame.timings.size(); event_id++)
		{
			float block_height = (frame.timings.at(event_id) / s_MaxHeight) * graph_height;

			const glm::vec2 margin{ 1.0f, -1.0f };
			const glm::vec2 offset{ frame_id * frame_width, graph_height - height_offset };

			glm::vec2 min_point = start + offset + margin;
			glm::vec2 max_point = min_point + glm::vec2(frame_width, -block_height) - margin;

			height_offset += block_height;

			ImU32 color = color_palette[frame.ids.at(event_id) % num_colors];

			drawList->AddRectFilled(ImVec2(min_point.x, min_point.y), ImVec2(max_point.x, max_point.y), color);
		}

	}

	glm::vec2 selected_min = start + glm::vec2(s_SelectedFrame * frame_width, graph_height);
	glm::vec2 selected_max = selected_min + glm::vec2(frame_width, - graph_height);

	drawList->AddRect(ImVec2(selected_min.x, selected_min.y), ImVec2(selected_max.x, selected_max.y), 0xffffffff);

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("CPU legend", ImVec2(0.0f, cpu_timing_height), false);

	auto& frame = s_Frames[s_SelectedFrame];

	ImGui::Text("CPU TIMINGS:");
	ImGui::Separator();

	for (size_t idx = 0; idx < frame.timings.size(); idx++)
	{
		auto id = frame.ids.at(idx);

		ImU32 color = color_palette[id % num_colors];

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::Text(("[" + std::to_string(frame.timings.at(idx)) + " ms] " + s_EventLabels.at(id)).c_str());
		ImGui::PopStyleColor();
	}

	ImGui::EndChild();

	ImGui::Columns(2, "###col");
	ImGuiUtils::Checkbox("Stop Profiling", &s_StopProfiling);
	ImGuiUtils::SliderInt("Selected Frame", &s_SelectedFrame, 0, s_NumFrames - 2);
	ImGuiUtils::SliderFloat("Max Height [ms]", &s_MaxHeight, 1.0f, 34.0f);
	ImGui::Columns(1, "###col");

	ImGui::End();
}


ProfilerCPUEvent::ProfilerCPUEvent(const std::string& name)
	: m_Start(std::chrono::high_resolution_clock::now())
	, m_ID(Profiler::GetEventID(name))
{

}

ProfilerCPUEvent::~ProfilerCPUEvent()
{
	using namespace std::chrono;

	auto current = high_resolution_clock::now();
	float time_ms = duration_cast<nanoseconds>(current - m_Start).count() * 1e-6;

	Profiler::SetEventTime(m_ID, time_ms);
}