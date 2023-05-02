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

void ConstIntTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Value);
}

ConstFloatTask::ConstFloatTask(const std::string& uniform_name, float val) 
    : UniformName(uniform_name), Value(val) {}

void ConstFloatTask::OnDispatch(Shader& shader, const InstanceData& data)
{
    auto value = std::get<float>(data);
    shader.setUniform1f(UniformName, value);
}

void ConstFloatTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Value);
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

void SliderFloatTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Def);
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

void ColorEdit3Task::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(Def);
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

void GLEnumTask::ProvideDefaultData(std::vector<InstanceData>& data)
{
    data.push_back(0);
}

//===========================================================================

void Procedure::CompileShader(const std::string& filepath) {
    m_Shader = std::unique_ptr<Shader>(new Shader(filepath));
}

void Procedure::OnDispatch(int res, const std::vector<InstanceData>& data) {
    m_Shader -> Bind();

    unsigned int i = 0;
    
    for (auto& task : m_Tasks) {
        task->OnDispatch(*m_Shader.get(), data[i]);

        ++i;
    }

    //Magic number "32" needs to be the same as local size
    //declared in the compute shader files
    glDispatchCompute(res / 32, res / 32, 1);
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

void EditorBase::RegisterShader(const std::string& name, 
                                    const std::string& filepath)
{
    if (m_Procedures.count(name)) return;

    m_Procedures[name].CompileShader(filepath);
}

//===========================================================================

void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                              std::vector<ProcedureInstance>& instances,
                              const std::string& name) {
    if (procedures.count(name)) {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures[name];
        auto& data = instances.back().Data;

        for (auto& task : procedure.m_Tasks) {
            task->ProvideDefaultData(data);
        }
    }
}

void OnDispatchImpl(std::unordered_map<std::string, Procedure>& procedures,
                    std::vector<ProcedureInstance>& instances,
                    int res) 
{
    for (auto& instance : instances) {
        auto& data = instance.Data;

        procedures[instance.Name].OnDispatch(res, data);
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

            res = res || procedures[instance.Name].OnImGui(data, id);

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

TextureEditor::TextureEditor(const std::string& name)
    : m_Name(name), m_InstanceID(s_InstanceCount++) {}

void TextureEditor::AddProcedureInstance(const std::string& name) {
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name);
}

void TextureEditor::OnDispatch(int res) {
    OnDispatchImpl(m_Procedures, m_Instances, res);
}

bool TextureEditor::OnImGui() {
    bool res =  OnImGuiImpl(m_Procedures, m_Instances, m_InstanceID);
    res |= PopupImpl(m_Procedures, m_Instances, m_Name, m_PopupOpen);

    return res;
}

unsigned int TextureEditor::s_InstanceCount = 0;

//===========================================================================

TextureArrayEditor::TextureArrayEditor(const std::string& name, int n)
    : m_Name(name), m_InstanceID(s_InstanceCount++)
{
    for (int i = 0; i < n; i++)
        m_InstanceLists.push_back(std::vector<ProcedureInstance>());
}

void TextureArrayEditor::AddProcedureInstance(int layer, const std::string& name) {
    auto& instances = m_InstanceLists[layer];

    AddProcedureInstanceImpl(m_Procedures, instances, name);
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

unsigned int TextureArrayEditor::s_InstanceCount = 0;