#pragma once

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <chrono>

struct FrameData {
	std::vector<float> timings;
	std::vector<size_t> ids;

	FrameData();
};

class Profiler {
public:
	static size_t GetEventID(const std::string& name);

	static void SetEventTime(size_t idx, float time);
	static void NextFrame();

	static void OnImGui(bool& open);

private:
	static const int s_NumFrames;
	static int s_SelectedFrame;
	static bool s_StopProfiling;
	static float s_MaxHeight;

	static std::vector<std::string> s_EventLabels;
	static std::deque<FrameData> s_Frames;
};


class ProfilerCPUEvent {
public:
	ProfilerCPUEvent(const std::string& name);
	~ProfilerCPUEvent();

private:
	size_t m_ID;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};