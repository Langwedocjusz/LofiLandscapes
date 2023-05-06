#include "Serializer.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

Serializer::Serializer()
    : m_CurrentPath(std::filesystem::current_path())
    , m_LoadDialogOpen(true), m_SaveDialogOpen(true)
    , m_LoadToBeOpened(false), m_SaveToBeOpened(false)
    , m_Filename("Your_File_Name.world")
{
    m_Filename.resize(m_MaxNameLength);
}

void Serializer::TriggerSave()
{
    m_SaveToBeOpened = true;
}

void Serializer::TriggerLoad()
{
    m_LoadToBeOpened = true;
}

void Serializer::OnImGui()
{
    if (m_LoadToBeOpened)
    {
        ImGui::OpenPopup("Load...");
        m_LoadToBeOpened = false;
    }

    else if (m_SaveToBeOpened)
    {
        ImGui::OpenPopup("Save...");
        m_SaveToBeOpened = false;
    }

    LoadPopup();
    SavePopup();

    m_LoadDialogOpen = true;
    m_SaveDialogOpen = true;
}

void Serializer::RegisterLoadCallback(const std::string& token, std::function<void(nlohmann::ordered_json&)> callback)
{
    if (m_LoadCallbacks.count(token) == 0)
        m_LoadCallbacks.insert(std::make_pair(token, callback));
    else
        std::cerr << "Serializer Error: Load callback for token " << token << " already registered\n";
}

void Serializer::RegisterSaveCallback(const std::string& token, std::function<void(nlohmann::ordered_json&)> callback)
{
    if (m_SaveCallbacks.count(token) == 0)
        m_SaveCallbacks.insert(std::make_pair(token, callback));
    else
        std::cerr << "Serializer Error: Save callback for token " << token << " already registered\n";
}

void Serializer::LoadPopup()
{
    if (ImGui::BeginPopupModal("Load...", &m_SaveDialogOpen)) {

        RenderFileBrowser();

        ImGuiUtils::Separator();

        //To-do: more 'rigorous' offset calculation
        const float width = ImGui::GetContentRegionAvail().x - 45.0f;

        ImGui::PushItemWidth(width);
        ImGui::InputText("##load_filename", m_Filename.data(), m_MaxNameLength, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("Load"))
        {
            Deserialize();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Serializer::SavePopup()
{
    if (ImGui::BeginPopupModal("Save...", &m_SaveDialogOpen)) {

        RenderFileBrowser();
        
        ImGuiUtils::Separator();

        //To-do: more 'rigorous' offset calculation
        const float width = ImGui::GetContentRegionAvail().x - 45.0f;

        ImGui::PushItemWidth(width);
        ImGui::InputText("##save_filename", m_Filename.data(), m_MaxNameLength);
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            Serialize();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Serializer::RenderFileBrowser()
{
    //To-do: more 'rigorous' offset calculation
    const float height = ImGui::GetContentRegionAvail().y - 60.0f;

    ImGui::BeginChild("#Filesystem browser", ImVec2(0.0f, height), true);

    if (ImGui::Selectable("..", true))
    {
        m_CurrentPath = m_CurrentPath.parent_path();
    }

    std::vector<std::filesystem::path> directories, files;

    for (const auto& entry : std::filesystem::directory_iterator(m_CurrentPath))
    {
        if (std::filesystem::is_directory(entry.path()))
            directories.push_back(entry.path());

        else
            files.push_back(entry.path());
    }

    for (const auto& path : directories)
    {
        if (ImGui::Selectable(path.string().c_str(), true))
            m_CurrentPath = path;
    }

    for (const auto& path : files)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(192, 192, 192, 255));

        if (ImGui::Selectable(path.string().c_str(), false))
            m_Filename = path.filename().string();

        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
}

void Serializer::Serialize()
{
    nlohmann::ordered_json json;

    for (const auto & [token, callback] : m_SaveCallbacks)
    {
        callback(json[token]);
    }

    auto path = m_CurrentPath / m_Filename;

    std::ofstream output(path, std::ios::trunc);

    const int indent = 4;

    output << json.dump(indent);
}

void Serializer::Deserialize()
{
    auto path = m_CurrentPath / m_Filename;

    std::ifstream input(path);

    if (input)
    {
        auto json = nlohmann::ordered_json::parse(input);

        for (auto& [key, value] : json.items())
        {
            if (m_LoadCallbacks.count(key))
                m_LoadCallbacks[key](value); 
        }
    }

}