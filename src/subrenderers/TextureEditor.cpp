#include "TextureEditor.h"

#include "glad/glad.h"

#include "imgui_internal.h"
#include "ImGuiUtils.h"

#include <algorithm>

Procedure::Procedure(ResourceManager& manager)
    : m_ResourceManager(manager)
{}

void Procedure::CompileShader(const std::string& filepath) {
    m_Shader = m_ResourceManager.RequestComputeShader(filepath);
}

void Procedure::OnDispatch(int res, const std::vector<InstanceData>& v_data) {
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

bool OnImGuiImpl(std::unordered_map<std::string, Procedure>& procedures,
                 std::vector<ProcedureInstance>& instances, unsigned int id)
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

            res = res || procedures.at(instance.Name).OnImGui(data, id);

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


bool PopupImpl(std::unordered_map<std::string, Procedure>& procedures,
               std::vector<ProcedureInstance>& instances,
               const std::string& editor_name, bool& popup_open) 
{
    bool res = false;

    std::string name = "Choose procedure##" + editor_name;

    if (ImGuiUtils::ButtonCentered(("Add procedure##" + editor_name).c_str())) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - 250.0f, io.DisplaySize.y * 0.5f - 250.0f));
        ImGui::SetNextWindowSize(ImVec2(500.0f, 500.0f));

        ImGui::OpenPopup(name.c_str());
    }

    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal(name.c_str(), &popup_open, popup_flags)) {

        for (auto& it : procedures)
        {
            if (ImGuiUtils::ButtonCentered(it.first.c_str()))
            {
                res = true;
                AddProcedureInstanceImpl(procedures, instances, it.first);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    popup_open = true;

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
    bool res = OnImGuiImpl(m_Procedures, m_Instances, m_InstanceID);
    res |= PopupImpl(m_Procedures, m_Instances, m_Name, m_PopupOpen);

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

    bool res = OnImGuiImpl(m_Procedures, instances, m_InstanceID);
    res |= PopupImpl(m_Procedures, instances, m_Name, m_PopupOpen);

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