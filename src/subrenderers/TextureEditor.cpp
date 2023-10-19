#include "TextureEditor.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiUtils.h"

#include <algorithm>

ConstIntTask::ConstIntTask(const std::string& uniform_name, int val) 
    : UniformName(uniform_name), Value(val) {}

void ConstIntTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<int>(data);
    shader.setUniform1i(UniformName, value);
}

void ConstIntTask::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    output["Value"] = std::get<int>(data);
}

void ConstIntTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Value);
}

void ConstIntTask::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
    data.push_back(input["Value"].get<int>());
}

ConstFloatTask::ConstFloatTask(const std::string& uniform_name, float val) 
    : UniformName(uniform_name), Value(val) {}

void ConstFloatTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<float>(data);
    shader.setUniform1f(UniformName, value);
}

void ConstFloatTask::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    output["Value"] = std::get<float>(data);
}

void ConstFloatTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Value);
}

void ConstFloatTask::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
    data.push_back(input["Value"].get<float>());
}

SliderIntTask::SliderIntTask(const std::string& uniform_name,
                             const std::string& ui_name,
                             int min, int max, int def)
    : UniformName(uniform_name), UiName(ui_name), Min(min), Max(max), Def(def)
{}

void SliderIntTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<int>(data);
    shader.setUniform1i(UniformName, value);
}

void SliderIntTask::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    output[UiName] = std::get<int>(data);
}

void SliderIntTask::OnImGui(InstanceData& data, bool& state, const std::string& suffix)
{
    int* ptr = &std::get<int>(data);
    int value = *ptr;

    ImGuiUtils::SliderInt(UiName, &value, Min, Max, suffix);

    if (value != *ptr) {
        *ptr = value;
        state = true;
    }
}

void SliderIntTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Def);
}

void SliderIntTask::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
    data.push_back(input[UiName].get<int>());
}

SliderFloatTask::SliderFloatTask(const std::string& uniform_name,
                                 const std::string& ui_name,
                                 float min, float max, float def)
    : UniformName(uniform_name), UiName(ui_name), Min(min), Max(max), Def(def)
{}

void SliderFloatTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<float>(data);
    shader.setUniform1f(UniformName, value);
}

void SliderFloatTask::OnImGui(InstanceData& data, bool& state, const std::string& suffix)
{
    float* ptr = &std::get<float>(data);
    float value = *ptr;

    ImGuiUtils::SliderFloat(UiName, &value, Min, Max, suffix);

    if (value != *ptr) {
        *ptr = value;
        state = true;
    }
}

void SliderFloatTask::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    output[UiName] = std::get<float>(data);
}

void SliderFloatTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Def);
}

void SliderFloatTask::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
    data.push_back(input[UiName].get<float>());
}

ColorEdit3Task::ColorEdit3Task(const std::string& uniform_name,
                               const std::string& ui_name,
                               glm::vec3 def)
    : UniformName(uniform_name), UiName(ui_name), Def(def)
{}

void ColorEdit3Task::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<glm::vec3>(data);
    shader.setUniform3f(UniformName, value);
}

void ColorEdit3Task::OnImGui(InstanceData& data, bool& state, const std::string& suffix)
{
    glm::vec3* ptr = &std::get<glm::vec3>(data);
    glm::vec3 value = *ptr;

    ImGuiUtils::ColorEdit3(UiName, &value, suffix);

    if (value != *ptr) {
        *ptr = value;
        state = true;
    }
}

void ColorEdit3Task::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    glm::vec3& value = std::get<glm::vec3>(data);
    output[UiName] = { value.x, value.y, value.z };
}

void ColorEdit3Task::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Def);
}

void ColorEdit3Task::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
   auto values = input[UiName].get<std::array<float, 3>>();
    data.push_back(glm::vec3(values[0], values[1], values[2]));
}

GLEnumTask::GLEnumTask(const std::string& uniform_name,
                       const std::string& ui_name,
                       const std::vector<std::string>& labels)
     : UniformName(uniform_name), UiName(ui_name), Labels(labels)
{}

void GLEnumTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<int>(data);
    shader.setUniform1i(UniformName, value);
}

void GLEnumTask::OnImGui(InstanceData& data, bool& state, const std::string& suffix)
{
    int* ptr = &std::get<int>(data);
    int value = *ptr;

    ImGuiUtils::Combo(UiName, Labels, value, suffix);

    if (value != *ptr) {
        *ptr = value;
        state = true;
    }
}

void GLEnumTask::OnSerialize(nlohmann::ordered_json& output, InstanceData data)
{
    output[UiName] = std::get<int>(data);
}

void GLEnumTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(0);
}

void GLEnumTask::ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input)
{
    data.push_back(input[UiName].get<int>());
}

//===========================================================================

Procedure::Procedure(ResourceManager& manager)
    : m_ResourceManager(manager)
{}

void Procedure::CompileShader(const std::string& filepath) {
    m_Shader = m_ResourceManager.RequestComputeShader(filepath);
}

void Procedure::OnDispatch(int res, const std::vector<InstanceData>& data) {
    m_Shader -> Bind();

    unsigned int i = 0;
    
    for (auto& task : m_Tasks) {
        task->OnDispatch(*m_Shader.get(), data[i]);

        ++i;
    }

    m_Shader->Dispatch(res, res, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

bool Procedure::OnImGui(std::vector<InstanceData>& data, unsigned int id) {
    const std::string suffix = std::to_string(id);

    bool res = false;
    unsigned int i = 0;

    for (auto& task : m_Tasks) {
        task->OnImGui(data[i], res, suffix);

        ++i;
    }

    return res;
}

//===========================================================================

ProcedureInstance::ProcedureInstance(const std::string& name) : Name(name) {}

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

void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                              std::vector<ProcedureInstance>& instances,
                              const std::string& name)
{
    if (procedures.count(name)) {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures.at(name);
        auto& data = instances.back().Data;

        for (auto& task : procedure.m_Tasks) {
            task->ProvideDefaultData(data);
        }
    }
}

void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                              std::vector<ProcedureInstance>& instances, 
                              const std::string& name, nlohmann::ordered_json& input) 
{
    if (procedures.count(name)) {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures.at(name);
        auto& data = instances.back().Data;

        for (auto& task : procedure.m_Tasks) {
            task->ProvideData(data, input);
        }
    }
}

void OnDispatchImpl(std::unordered_map<std::string, Procedure>& procedures,
                    std::vector<ProcedureInstance>& instances,
                    int res) 
{
    for (auto& instance : instances) {
        auto& data = instance.Data;

        procedures.at(instance.Name).OnDispatch(res, data);
    }
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

        bool uncolapsed = ImGui::CollapsingHeader(name.c_str(), &instance.KeepAlive, flags);

        //To-do: rigorous computation of "offset"
        const float offset = 70.0f;

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - offset);

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

    if (ImGuiUtils::Button(("Add procedure##" + editor_name).c_str())) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - 250.0f, io.DisplaySize.y * 0.5f - 250.0f));
        ImGui::SetNextWindowSize(ImVec2(500.0f, 500.0f));

        ImGui::OpenPopup(name.c_str());
    }

    ImGuiWindowFlags popup_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginPopupModal(name.c_str(), &popup_open, popup_flags)) {

        for (auto& it : procedures)
        {
            if (ImGuiUtils::Button(it.first.c_str()))
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

#include <iostream>

TextureEditor::TextureEditor(ResourceManager& manager, const std::string& name)
    : EditorBase(manager), m_Name(name), m_InstanceID(s_InstanceCount++) 
{}

void TextureEditor::AddProcedureInstance(const std::string& name) {
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name);
}

void TextureEditor::AddProcedureInstance(const std::string& name, nlohmann::ordered_json& input) {
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name, input);
}

void TextureEditor::OnDispatch(int res) {
    OnDispatchImpl(m_Procedures, m_Instances, res);
}

bool TextureEditor::OnImGui() {
    bool res =  OnImGuiImpl(m_Procedures, m_Instances, m_InstanceID);
    res |= PopupImpl(m_Procedures, m_Instances, m_Name, m_PopupOpen);

    return res;
}

void TextureEditor::OnSerialize(nlohmann::ordered_json& output) {
    size_t instance_idx = 0;

    for (const auto& instance : m_Instances)
    {
        if (!m_Procedures.count(instance.Name))
            continue;

        const auto& procedure = m_Procedures.at(instance.Name);

        size_t task_idx = 0;

        for (auto& task : procedure.m_Tasks)
        {
            const std::string name = std::to_string(instance_idx) + "_" + instance.Name;
            task->OnSerialize(output[m_Name][name], instance.Data[task_idx]);

            task_idx++;
        }

        instance_idx++;
    }
}

void TextureEditor::OnDeserialize(nlohmann::ordered_json& input) {
    m_Instances.clear();
    
    for (auto& [key, value] : input.items())
    {
        const std::string name = key.substr(key.find_first_of("_") + 1);
        AddProcedureInstance(name, value);
    }
}

unsigned int TextureEditor::s_InstanceCount = 0;

//===========================================================================

TextureArrayEditor::TextureArrayEditor(ResourceManager& manager, const std::string& name, int n)
    : EditorBase(manager), m_Name(name), m_InstanceID(s_InstanceCount++)
{
    for (int i = 0; i < n; i++)
        m_InstanceLists.push_back(std::vector<ProcedureInstance>());
}

void TextureArrayEditor::AddProcedureInstance(int layer, const std::string& name) {
    auto& instances = m_InstanceLists[layer];

    AddProcedureInstanceImpl(m_Procedures, instances, name);
}

void TextureArrayEditor::AddProcedureInstance(int layer, const std::string& name, nlohmann::ordered_json& input) {
    auto& instances = m_InstanceLists[layer];

    AddProcedureInstanceImpl(m_Procedures, instances, name, input);
}

void TextureArrayEditor::OnDispatch(int layer, int res) {
    auto& instances = m_InstanceLists[layer];

    OnDispatchImpl(m_Procedures, instances, res);
}

bool TextureArrayEditor::OnImGui(int layer) {
    auto& instances = m_InstanceLists[layer];

    bool res = OnImGuiImpl(m_Procedures, instances, m_InstanceID);
    res |= PopupImpl(m_Procedures, instances, m_Name, m_PopupOpen);
    
    return res;
}

void TextureArrayEditor::OnSerialize(nlohmann::ordered_json& output) {
    for (size_t id = 0; id < m_InstanceLists.size(); id++)
    {
        size_t instance_idx = 0;

        for (const auto& instance : m_InstanceLists[id])
        {
            if (!m_Procedures.count(instance.Name))
                continue;

            const auto& procedure = m_Procedures.at(instance.Name);

            size_t task_idx = 0;

            for (auto& task : procedure.m_Tasks)
            {
                const std::string name = std::to_string(instance_idx) + "_" + instance.Name;

                task->OnSerialize(output[m_Name][std::to_string(id)][name], instance.Data[task_idx]);

                task_idx++;
            }

            instance_idx++;
        }
    }
}

void TextureArrayEditor::OnDeserialize(nlohmann::ordered_json& input) {
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

unsigned int TextureArrayEditor::s_InstanceCount = 0;
