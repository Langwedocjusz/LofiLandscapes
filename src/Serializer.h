#pragma once

#include <filesystem>

class Serializer {
public:
	Serializer();

	void TriggerSave();
	void TriggerLoad();

	void OnImGui();

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
};