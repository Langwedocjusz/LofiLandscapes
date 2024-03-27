#include "TextureEditor.h"

#include "glad/glad.h"

#include "imgui_internal.h"
#include "ImGuiUtils.h"

#include <algorithm>

Procedure::Procedure(ResourceManager& manager)
    : m_ResourceManager(manager)
{}

void Procedure::CompileShader(const std::string& filepath) 
{
    m_Shader = m_ResourceManager.RequestComputeShader(filepath);
}

void Procedure::OnDispatch(int res, const std::vector<InstanceData>& v_data) 
{
    m_Shader->Bind();

    for (size_t i = 0; i < m_Tasks.size(); i++)
    {
        auto& task = m_Tasks[i];
        auto& data = v_data[i];

        task->OnDispatch(*m_Shader.get(), data);
    }

    m_Shader->Dispatch(res, res, 1);

    //All procedure shaders read with imageLoad and save with imageStore
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

bool Procedure::OnImGui(std::vector<InstanceData>& v_data, uint32_t id)
{
    const std::string suffix = std::to_string(id);

    bool res = false;

    for (size_t i = 0; i < m_Tasks.size(); i++)
    {
        auto& task = m_Tasks[i];
        auto& data = v_data[i];

        task->OnImGui(data, res, suffix);
    }

    return res;
}

//===========================================================================

ProcedureInstance::ProcedureInstance(const std::string& name) 
    : Name(name) 
{

}

//===========================================================================

EditorBase::EditorBase(ResourceManager& manager)
    : m_ResourceManager(manager)
{}

void EditorBase::RegisterShader(const std::string& name, 
                                const std::string& filepath)
{
    if (m_Procedures.count(name)) return;

    m_Procedures.emplace(name, m_ResourceManager);
    m_Procedures.at(name).CompileShader(filepath);

    m_ProcedureNames.push_back(name);
    std::sort(m_ProcedureNames.begin(), m_ProcedureNames.end());
}

//===========================================================================

static void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                                     std::vector<ProcedureInstance>& instances,
                                     const std::string& name)
{
    if (procedures.count(name))
    {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures.at(name);
        auto& data = instances.back().Data;

        for (auto& task : procedure.m_Tasks)
        {
            task->ProvideDefaultData(data);
        }
    }
}

static void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                                     std::vector<ProcedureInstance>& instances,
                                     const std::string& name, nlohmann::ordered_json& input)
{
    if (procedures.count(name))
    {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures.at(name);
        auto& data = instances.back().Data;

        for (auto& task : procedure.m_Tasks)
        {
            task->ProvideData(data, input);
        }
    }
}

static void OnDispatchImpl(std::unordered_map<std::string, Procedure>& procedures,
                           std::vector<ProcedureInstance>& instances,
                           int res)
{
    for (auto& instance : instances)
    {
        auto& data = instance.Data;

        procedures.at(instance.Name).OnDispatch(res, data);
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}


static bool AddProcedureButtonImpl(std::vector<std::string>& names,
                                   std::unordered_map<std::string, Procedure>& procedures,
                                   std::vector<ProcedureInstance>& instances,
                                   uint32_t id)
{
    bool state_changed = false;

    size_t selected_id = 0;

    const std::string name = "Add procedure:";

    ImGui::Columns(2, "###col", false);
    ImGui::TextUnformatted(name.c_str());
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);

    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Button]);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_ButtonHovered]);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.Colors[ImGuiCol_ButtonActive]);

    if (ImGui::BeginCombo(("##"+name+std::to_string(id)).c_str(), names.at(selected_id).c_str()))
    {
        for (size_t n = 0; n < names.size(); n++)
        {
            bool is_selected = (selected_id == n);

            if (ImGui::Selectable(names.at(n).c_str(), is_selected))
            {
                selected_id = n;
                AddProcedureInstanceImpl(procedures, instances, names.at(selected_id));
                state_changed = true;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleColor(3);

    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::Columns(1, "###col");

    return state_changed;
}

static bool OnImGuiImpl(std::unordered_map<std::string, Procedure>& procedures,
                        std::vector<ProcedureInstance>& instances, uint32_t id)
{
    bool res = false;

    for (int i = 0; i < instances.size(); i++) {
        auto& instance = instances[i];
        auto& data = instance.Data;

        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowItemOverlap;
        const std::string name = instance.Name + "##" + std::to_string(id) + std::to_string(i);

        ImVec2 initial_pos = ImGui::GetCursorPos();

        ImVec2 initial_pos_screen = ImGui::GetCursorScreenPos();

        bool uncolapsed = ImGui::CollapsingHeader(name.c_str(), &instance.KeepAlive, flags);

        const float header_end = ImGui::GetContentRegionAvail().x;

        const float offset = 80.0f; //To-do: rigorous computation of "offset"

        ImGui::SameLine();
        ImGui::SetCursorPosX(initial_pos.x + header_end - offset);

        if (ImGui::ArrowButton((name + "up").c_str(), 2) && i>0)
        {
            std::iter_swap(instances.begin() + i - 1, instances.begin() + i);
            res = true;
        }

        ImGui::SameLine();

        if (ImGui::ArrowButton((name + "down").c_str(), 3) && i < instances.size() - 1)
        {
            std::iter_swap(instances.begin() + i, instances.begin() + i + 1);
            res = true;
        }

        if (uncolapsed)
        {
            ImGui::Columns(2, "###col");
            ImGui::PushID(i);

            res |= procedures.at(instance.Name).OnImGui(data, id);

            ImGui::PopID();
            ImGui::Columns(1, "###col");
        }

        if (!instance.KeepAlive) {
            instances.erase(instances.begin() + i);
            res = true;
        }

        ImGui::Spacing();
    }

    return res;
}

//===========================================================================

uint32_t TextureEditor::s_InstanceCount = 0;

TextureEditor::TextureEditor(ResourceManager& manager, const std::string& name)
    : EditorBase(manager), m_Name(name), m_InstanceID(s_InstanceCount++)
{

}

void TextureEditor::AddProcedureInstance(const std::string& name)
{
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name);
}

void TextureEditor::AddProcedureInstance(const std::string& name, nlohmann::ordered_json& input)
{
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name, input);
}

void TextureEditor::OnDispatch(int res)
{
    OnDispatchImpl(m_Procedures, m_Instances, res);
}

bool TextureEditor::OnImGui()
{
    bool res = AddProcedureButtonImpl(m_ProcedureNames, m_Procedures, m_Instances, m_InstanceID);
    res |= OnImGuiImpl(m_Procedures, m_Instances, m_InstanceID);

    return res;
}

void TextureEditor::OnSerialize(nlohmann::ordered_json& output)
{
    for (size_t instance_idx = 0; instance_idx < m_Instances.size(); instance_idx++)
    {
        auto& instance = m_Instances[instance_idx];

        if (!m_Procedures.count(instance.Name))
            continue;

        const auto& procedure = m_Procedures.at(instance.Name);

        const std::string name = std::to_string(instance_idx) + "_" + instance.Name;

        for (size_t task_idx = 0; task_idx < procedure.m_Tasks.size(); task_idx++)
        {
            auto& task = procedure.m_Tasks[task_idx];

            task->OnSerialize(output[m_Name][name], instance.Data[task_idx]);
        }
    }
}

void TextureEditor::OnDeserialize(nlohmann::ordered_json& input)
{
    m_Instances.clear();

    for (auto& [key, value] : input.items())
    {
        const std::string name = key.substr(key.find_first_of("_") + 1);
        AddProcedureInstance(name, value);
    }
}

//===========================================================================

uint32_t TextureArrayEditor::s_InstanceCount = 0;

TextureArrayEditor::TextureArrayEditor(ResourceManager& manager, const std::string& name, int n)
    : EditorBase(manager), m_Name(name), m_InstanceID(s_InstanceCount++)
{
    for (int i = 0; i < n; i++)
        m_InstanceLists.push_back(std::vector<ProcedureInstance>());
}

void TextureArrayEditor::AddProcedureInstance(int layer, const std::string& name)
{
    auto& instances = m_InstanceLists[layer];

    AddProcedureInstanceImpl(m_Procedures, instances, name);
}

void TextureArrayEditor::AddProcedureInstance(int layer, const std::string& name, nlohmann::ordered_json& input)
{
    auto& instances = m_InstanceLists[layer];

    AddProcedureInstanceImpl(m_Procedures, instances, name, input);
}

void TextureArrayEditor::OnDispatch(int layer, int res)
{
    auto& instances = m_InstanceLists[layer];

    OnDispatchImpl(m_Procedures, instances, res);
}

bool TextureArrayEditor::OnImGui(int layer)
{
    auto& instances = m_InstanceLists[layer];

    bool res = AddProcedureButtonImpl(m_ProcedureNames, m_Procedures, instances, m_InstanceID);
    res |= OnImGuiImpl(m_Procedures, instances, m_InstanceID);

    return res;
}

void TextureArrayEditor::OnSerialize(nlohmann::ordered_json& output)
{
    for (size_t id = 0; id < m_InstanceLists.size(); id++)
    {
        for (size_t instance_idx = 0; instance_idx < m_InstanceLists[id].size(); instance_idx++)
        {
            const auto& instance = m_InstanceLists.at(id).at(instance_idx);

            if (!m_Procedures.count(instance.Name))
                continue;

            const auto& procedure = m_Procedures.at(instance.Name);

            const std::string name = std::to_string(instance_idx) + "_" + instance.Name;

            for (size_t task_idx = 0; task_idx < procedure.m_Tasks.size(); task_idx++)
            {
                auto& task = procedure.m_Tasks[task_idx];

                task->OnSerialize(output[m_Name][std::to_string(id)][name], instance.Data[task_idx]);
            }
        }
    }
}

void TextureArrayEditor::OnDeserialize(nlohmann::ordered_json& input)
{
    m_InstanceLists.clear();

    size_t idx = 0;

    for (auto& [layer, subinput] : input.items())
    {
        m_InstanceLists.push_back(std::vector<ProcedureInstance>());

        for (auto& [key, value] : subinput.items())
        {
            const std::string name = key.substr(key.find_first_of("_") + 1);

            AddProcedureInstance(idx, name, value);
        }

        idx++;
    }
}