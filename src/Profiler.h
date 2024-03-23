#pragma once

//CPU/GPU Visual Profiler inspired by one created by Alexander Sannikov:
//https://youtu.be/4qzILPJrDzo
//Usage of OpenGL timer queries to time GPU based on:
//https://www.lighthouse3d.com/tutorials/opengl-timer-query/

#include <string>
#include <vector>
#include <deque>
#include <chrono>

#include "imgui.h"

struct FrameData {
	std::vector<float> Timings;
	std::vector<size_t> Ids;

	FrameData();
};

class Profiler {
public:
	static size_t GetCPUEventID(const std::string& name);
	static size_t GetGPUEventID(const std::string& name);

	static uint32_t GetFrontbufferQueryID(size_t id);
	static uint32_t GetBackbufferQueryID(size_t id);

	static void NextFrame();
	static void SwapBuffers();

	static void SubmitCpuEvent(size_t idx, float time);
	static void SubmitGpuEvent(size_t idx);

	static void OnInit();
	static void OnImGui(bool& open);

private:
	static void DrawGraph(std::deque<FrameData>& frames, const ImU32* color_palette, int palette_size);
	static void DrawLegend(std::deque<FrameData>& frames, std::vector<std::string>& labels,
		                   const std::string& title, const ImU32* color_palette, int palette_size);

	static const int s_NumFrames = 48;

	static int s_SelectedFrame;
	static bool s_StopProfiling;
	static float s_MaxHeight;

	static std::deque<FrameData> s_CPUFrames, s_GPUFrames;
	static std::vector<std::string> s_CPUEventLabels, s_GPUEventLabels;

	static const int s_MaxGPUQueries = 32;

	static std::vector<uint32_t> s_QueryIDs1, s_QueryIDs2;
	static std::vector<uint32_t> *s_FrontBuffer, *s_BackBuffer;
};


class ProfilerCPUEvent {
public:
	ProfilerCPUEvent(const std::string& name);
	~ProfilerCPUEvent();

private:
	size_t m_ID;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class ProfilerGPUEvent {
public:
	ProfilerGPUEvent(const std::string& name);
	~ProfilerGPUEvent();

private:
	size_t m_ID;
};