#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <functional>

class Serializer {
public:
	Serializer();

	void TriggerSave();
	void TriggerLoad();

	void OnImGui();

	void RegisterSaveCallback(const std::string& token, std::function<void(std::ofstream&)> callback);

private:
	void LoadPopup();
	void SavePopup();

	void RenderFileBrowser();

	void Serialize();
	void Deserialize();

	std::filesystem::path m_CurrentPath;

	bool m_LoadDialogOpen, m_SaveDialogOpen;
	bool m_LoadToBeOpened, m_SaveToBeOpened;

	//Arbitrary, To-do: make an informed decision about this
	const size_t m_MaxNameLength = 40;
	std::string m_Filename;

	std::map<std::string, std::function<void(std::ofstream&)>> m_SaveCallbacks;
};