#include "Profiler.h"

#include "imgui.h"
#include "glad/glad.h"
#include "glm/glm.hpp"

#include "ImGuiUtils.h"

#include <algorithm>

FrameData::FrameData()
{
	const int start_capacity = 10;

	Timings.reserve(start_capacity);
	Ids.reserve(start_capacity);
}

//=====Profiler static variables=====================

int Profiler::s_SelectedFrame = s_NumFrames - 2;
bool Profiler::s_StopProfiling = false;
float Profiler::s_MaxHeight = 17.0f;

std::vector<std::string> Profiler::s_CPUEventLabels, Profiler::s_GPUEventLabels;
std::deque<FrameData> Profiler::s_CPUFrames, Profiler::s_GPUFrames;

std::vector<uint32_t> Profiler::s_QueryIDs1, Profiler::s_QueryIDs2;

std::vector<uint32_t>* Profiler::s_FrontBuffer = &s_QueryIDs1;
std::vector<uint32_t>* Profiler::s_BackBuffer = &s_QueryIDs2;

//===================================================

size_t FindOrInsert(std::vector<std::string>& vec, const std::string& name)
{
	auto idx = std::find(vec.begin(), vec.end(), name);

	if (idx != vec.end())
	{
		return std::distance(vec.begin(), idx);
	}

	else
	{
		vec.push_back(name);
		return vec.size() - 1;
	}
}

size_t Profiler::GetCPUEventID(const std::string& name)
{
	return FindOrInsert(s_CPUEventLabels, name);
}

size_t Profiler::GetGPUEventID(const std::string& name)
{
	return FindOrInsert(s_GPUEventLabels, name);
}

void Profiler::SubmitCpuEvent(size_t idx, float time)
{
	if (s_StopProfiling) return;

	if (!s_CPUFrames.empty())
	{
		s_CPUFrames.back().Timings.push_back(time);
		s_CPUFrames.back().Ids.push_back(idx);
	}
}

void Profiler::SubmitGpuEvent(size_t idx)
{
	if (s_StopProfiling) return;

	if (!s_GPUFrames.empty())
	{
		s_GPUFrames.back().Ids.push_back(idx);
	}
}

void Profiler::NextFrame()
{
	if (s_StopProfiling) return;

	//Retrieve gpu timings from last frame
	if (!s_GPUFrames.empty())
	{
		auto& last_frame = s_GPUFrames.back();

		for (size_t event_id = 0; event_id < last_frame.Ids.size(); event_id++)
		{
			auto id = last_frame.Ids[event_id];
			auto query_id = GetFrontbufferQueryID(id);

			size_t time;
			glGetQueryObjectui64v(query_id, GL_QUERY_RESULT, &time);

			float time_ms = float(time) * 1e-6;

			last_frame.Timings.push_back(time_ms);
		}
	}

	//Cycle frames
	if (s_CPUFrames.size() >= s_NumFrames)
		s_CPUFrames.pop_front();

	if (s_GPUFrames.size() >= s_NumFrames)
		s_GPUFrames.pop_front();

	s_CPUFrames.push_back(FrameData());
	s_GPUFrames.push_back(FrameData());
}

void Profiler::SwapBuffers()
{
	std::swap(s_FrontBuffer, s_BackBuffer);
}

void Profiler::OnInit()
{
	s_QueryIDs1.resize(s_MaxGPUQueries);
	s_QueryIDs2.resize(s_MaxGPUQueries);

	glGenQueries(s_MaxGPUQueries, &s_QueryIDs1[0]);
	glGenQueries(s_MaxGPUQueries, &s_QueryIDs2[0]);
}

uint32_t Profiler::GetFrontbufferQueryID(size_t id)
{
	return s_FrontBuffer->at(id);
}

uint32_t Profiler::GetBackbufferQueryID(size_t id)
{
	return s_BackBuffer->at(id);
}

void Profiler::DrawGraph(std::deque<FrameData>& frames, const ImU32* color_palette, int palette_size)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	auto ImToGlm = [](ImVec2 v) {return glm::vec2(v.x, v.y); };

	const glm::vec2 start = ImToGlm(ImGui::GetCursorScreenPos());
	const glm::vec2 graph_size = ImToGlm(ImGui::GetContentRegionAvail());
	const float graph_width = graph_size.x, graph_height = graph_size.y;

	const float frame_width = graph_width / (s_NumFrames - 1);

	for (size_t frame_id = 0; frame_id < frames.size() - 1; frame_id++)
	{
		auto& frame = frames.at(frame_id);

		float height_offset = 0.0f;

		for (size_t event_id = 0; event_id < frame.Timings.size(); event_id++)
		{
			const float block_height = (frame.Timings.at(event_id) / s_MaxHeight) * graph_height;

			const glm::vec2 margin{ 1.0f, -1.0f };
			const glm::vec2 offset{ frame_id * frame_width, graph_height - height_offset };

			glm::vec2 min_point = start + offset + margin;
			glm::vec2 max_point = min_point + glm::vec2(frame_width, -block_height) - margin;

			height_offset += block_height;

			const ImU32 color = color_palette[frame.Ids.at(event_id) % palette_size];

			drawList->AddRectFilled(ImVec2(min_point.x, min_point.y), ImVec2(max_point.x, max_point.y), color);
		}

	}

	const glm::vec2 selected_min = start + glm::vec2(s_SelectedFrame * frame_width, graph_height);
	const glm::vec2 selected_max = selected_min + glm::vec2(frame_width, -graph_height);

	drawList->AddRect(ImVec2(selected_min.x, selected_min.y), ImVec2(selected_max.x, selected_max.y), 0xffffffff);
}

void Profiler::DrawLegend(std::deque<FrameData>& frames, std::vector<std::string>& labels,
	                      const std::string& title, const ImU32* color_palette, int palette_size)
{
	auto& frame = frames[s_SelectedFrame];

	ImGui::TextUnformatted(title.c_str());
	ImGui::Separator();

	for (size_t idx = 0; idx < frame.Timings.size(); idx++)
	{
		auto id = frame.Ids.at(idx);

		const ImU32 color = color_palette[id % palette_size];

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextUnformatted(
			("[" + std::to_string(frame.Timings.at(idx)) + " ms] " + labels.at(id)).c_str()
		);
		ImGui::PopStyleColor();
	}
}

void Profiler::OnImGui(bool& open)
{
	ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f), ImGuiCond_FirstUseEver);

	ImGui::Begin("Profiler", &open);

	const int palette_size = 6;
	const ImU32 color_palette[palette_size]{
		//https://flatuicolors.com/palette/defo
		IM_COL32(26, 188, 156, 255),
		IM_COL32(46, 204, 113, 255),
		IM_COL32(52, 152, 219, 255),
		IM_COL32(155, 89, 182, 255),
		IM_COL32(52, 73, 94, 255),
		IM_COL32(22, 160, 133, 255)
	};

	const float legend_width = 250.0f, min_graph_width = 200.0f, min_graph_height = 150.0f;
	const float graph_width = std::max(min_graph_width, ImGui::GetContentRegionAvail().x - legend_width);
	const float graph_height = std::max(min_graph_height, 0.5f * ImGui::GetContentRegionAvail().y);

	ImGui::BeginChild("CPU Timing", ImVec2(graph_width, graph_height), true);
	DrawGraph(s_CPUFrames, color_palette, palette_size);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("CPU Legend", ImVec2(0.0f, graph_height), false);
	DrawLegend(s_CPUFrames, s_CPUEventLabels, "CPU TIMINGS", color_palette, palette_size);
	ImGui::EndChild();

	ImGui::BeginChild("GPU Timing", ImVec2(graph_width, graph_height), true);
	DrawGraph(s_GPUFrames, color_palette, palette_size);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("GPU Legend", ImVec2(0.0f, graph_height), false);
	DrawLegend(s_GPUFrames, s_GPUEventLabels, "GPU TIMINGS", color_palette, palette_size);
	ImGui::EndChild();

	ImGui::Columns(2, "###col");
	ImGuiUtils::ColCheckbox("Stop Profiling", &s_StopProfiling);
	ImGuiUtils::ColSliderInt("Selected Frame", &s_SelectedFrame, 0, s_NumFrames - 2);
	ImGuiUtils::ColSliderFloat("Max Height [ms]", &s_MaxHeight, 1.0f, 34.0f);
	ImGui::Columns(1, "###col");

	ImGui::End();
}


ProfilerCPUEvent::ProfilerCPUEvent(const std::string& name)
	: m_Start(std::chrono::high_resolution_clock::now())
	, m_ID(Profiler::GetCPUEventID(name))
{

}

ProfilerCPUEvent::~ProfilerCPUEvent()
{
	using namespace std::chrono;

	const auto current = high_resolution_clock::now();
	const float time_ms = duration_cast<nanoseconds>(current - m_Start).count() * 1e-6;

	Profiler::SubmitCpuEvent(m_ID, time_ms);
}


ProfilerGPUEvent::ProfilerGPUEvent(const std::string& name)
	: m_ID(Profiler::GetGPUEventID(name))
{
	glBeginQuery(GL_TIME_ELAPSED, Profiler::GetBackbufferQueryID(m_ID));
}

ProfilerGPUEvent::~ProfilerGPUEvent()
{
	glEndQuery(GL_TIME_ELAPSED);

	Profiler::SubmitGpuEvent(m_ID);
}
