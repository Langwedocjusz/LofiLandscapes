#pragma once

#include <fstream>
#include <filesystem>

#include <map>
#include <functional>

#include "nlohmann/json.hpp"

class Serializer {
public:
	Serializer();

	void TriggerSave();
	void TriggerLoad();

	void OnImGui();

	void RegisterLoadCallback(const std::string& token, std::function<void(nlohmann::ordered_json&)> callback);
	void RegisterSaveCallback(const std::string& token, std::function<void(nlohmann::ordered_json&)> callback);

private:
	void LoadPopup();
	void SavePopup();

	void RenderFileBrowser(float lower_margin);

	void Serialize();
	void Deserialize();

	std::filesystem::path m_CurrentPath;

	bool m_LoadDialogOpen, m_SaveDialogOpen;
	bool m_LoadToBeOpened, m_SaveToBeOpened;

	//Arbitrary, To-do: make an informed decision about this
	const size_t m_MaxNameLength = 40;
	std::string m_Filename;

	std::map<std::string, std::function<void(nlohmann::ordered_json&)>> m_LoadCallbacks;
	std::map<std::string, std::function<void(nlohmann::ordered_json&)>> m_SaveCallbacks;
};