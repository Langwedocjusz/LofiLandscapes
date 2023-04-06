#include "TextureEditor.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiUtils.h"

#include <algorithm>

ConstIntTask::ConstIntTask(const std::string& uniform_name, int val) 
    : UniformName(uniform_name), Value(val) {}

ConstFloatTask::ConstFloatTask(const std::string& uniform_name, float val) 
    : UniformName(uniform_name), Value(val) {}

SliderIntTask::SliderIntTask(const std::string& uniform_name,
                             const std::string& ui_name,
                             int min, int max, int def)
    : UniformName(uniform_name), UiName(ui_name), Min(min), Max(max), Def(def)
{}


SliderFloatTask::SliderFloatTask(const std::string& uniform_name,
                                 const std::string& ui_name,
                                 float min, float max, float def)
    : UniformName(uniform_name), UiName(ui_name), Min(min), Max(max), Def(def)
{}

ColorEdit3Task::ColorEdit3Task(const std::string& uniform_name,
                               const std::string& ui_name,
                               glm::vec3 def)
    : UniformName(uniform_name), UiName(ui_name), Def(def)
{}

GLEnumTask::GLEnumTask(const std::string& uniform_name,
                       const std::string& ui_name,
                       const std::vector<std::string>& labels)
     : UniformName(uniform_name), UiName(ui_name), m_Labels(labels)
{}

//===========================================================================

void Procedure::CompileShader(const std::string& filepath) {
    m_Shader = std::unique_ptr<Shader>(new Shader(filepath));
}

void Procedure::AddConstInt(const std::string& uniform_name, int def_val) {
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new ConstIntTask(uniform_name, def_val)
    ));
}

void Procedure::AddConstFloat(const std::string& uniform_name, float def_val) {
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new ConstFloatTask(uniform_name, def_val)
    ));
}

void Procedure::AddSliderInt(const std::string& uniform_name, 
                             const std::string& ui_name,
                             int min_val, int max_val, int def_val)
{
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new SliderIntTask(uniform_name, ui_name, min_val, max_val, def_val)
    ));
}

void Procedure::AddSliderFloat(const std::string& uniform_name, 
                               const std::string& ui_name,
                               float min_val, float max_val, float def_val)
{
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new SliderFloatTask(uniform_name, ui_name, min_val, max_val, def_val)
    ));
}

void Procedure::AddColorEdit3(const std::string& uniform_name, 
                              const std::string& ui_name,
                              glm::vec3 def_val)
{
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new ColorEdit3Task(uniform_name, ui_name, def_val)
    ));
}

void Procedure::AddGLEnum(const std::string& uniform_name,
                          const std::string& ui_name,
                          const std::vector<std::string>& labels)
{
    m_Tasks.push_back(std::unique_ptr<EditorTask>(
        new GLEnumTask(uniform_name, ui_name, labels)
    ));
}

void Procedure::OnDispatch(int res, const std::vector<InstanceData>& data) {
    m_Shader -> Bind();

    unsigned int i = 0;
    
    for (auto& task : m_Tasks) {
        auto type = task->getType();

        switch(type) {
            case TaskType::ConstInt:
            {
                ConstIntTask* typed_task =
                    dynamic_cast<ConstIntTask*>(task.get());

                auto value = std::get<int>(data[i]);
                m_Shader -> setUniform1i(typed_task->UniformName, value);
                break;
            }
            case TaskType::ConstFloat:
            {
                ConstFloatTask* typed_task =
                    dynamic_cast<ConstFloatTask*>(task.get());

                auto value = std::get<float>(data[i]);
                m_Shader -> setUniform1f(typed_task->UniformName, value);
                break;
            }
            case TaskType::SliderInt:
            {
                SliderIntTask* typed_task = 
                    dynamic_cast<SliderIntTask*>(task.get());

                auto value = std::get<int>(data[i]);
                m_Shader -> setUniform1i(typed_task->UniformName, value);
                break;
            }
            case TaskType::SliderFloat:
            {
                SliderFloatTask* typed_task = 
                    dynamic_cast<SliderFloatTask*>(task.get());

                auto value = std::get<float>(data[i]);
                m_Shader -> setUniform1f(typed_task->UniformName, value);
                break;
            }
            case TaskType::ColorEdit3:
            {
                ColorEdit3Task* typed_task = 
                    dynamic_cast<ColorEdit3Task*>(task.get());

                auto value = std::get<glm::vec3>(data[i]);
                m_Shader -> setUniform3f(typed_task->UniformName, value);
                break;
            }
            case TaskType::GLEnum:
            {
                GLEnumTask* typed_task =
                    dynamic_cast<GLEnumTask*>(task.get());

                auto value = std::get<int>(data[i]);
                m_Shader->setUniform1i(typed_task->UniformName, value);
                break;
            }
            default:
            {
                break;
            }
        }

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
        auto type = task->getType();

        switch(type) {
            case TaskType::SliderInt:
            {
                SliderIntTask* typed_task = 
                    dynamic_cast<SliderIntTask*>(task.get());

                int* ptr = &std::get<int>(data[i]);
                int value = *ptr;

                ImGuiUtils::SliderInt((typed_task->UiName), &value, 
                                       typed_task->Min, typed_task->Max, suffix);

                if (value != *ptr) {
                    *ptr = value;
                    res = true;
                }

                break;
            }
            case TaskType::SliderFloat:
            {
                SliderFloatTask* typed_task = 
                    dynamic_cast<SliderFloatTask*>(task.get());

                float* ptr = &std::get<float>(data[i]);
                float value = *ptr;

                ImGuiUtils::SliderFloat((typed_task->UiName), &value, 
                                         typed_task->Min, typed_task->Max, suffix);

                if (value != *ptr) {
                    *ptr = value;
                    res = true;
                }

                break;
            }
            case TaskType::ColorEdit3:
            {
                ColorEdit3Task* typed_task = 
                    dynamic_cast<ColorEdit3Task*>(task.get());

                glm::vec3* ptr = &std::get<glm::vec3>(data[i]);
                glm::vec3 value = *ptr;

                ImGuiUtils::ColorEdit3((typed_task->UiName), &value, suffix);

                if (value != *ptr) {
                    *ptr = value;
                    res = true;
                }

                break;
            }
            case TaskType::GLEnum:
            {
                GLEnumTask* typed_task =
                    dynamic_cast<GLEnumTask*>(task.get());

                int* ptr = &std::get<int>(data[i]);
                int value = *ptr;

                std::string& name = typed_task->UiName;
                std::vector<std::string>& labels = typed_task->m_Labels;

                ImGuiUtils::Combo(name, labels, value, suffix);

                if (value != *ptr) {
                    *ptr = value;
                    res = true;
                }

                break;
            }
            default:
            {
                break;
            }
        }

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

void EditorBase::AttachConstInt(const std::string& name,
                                    const std::string& uniform_name,
                                    int def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddConstInt(uniform_name, def_val);
}
    
void EditorBase::AttachConstFloat(const std::string& name,
                                      const std::string& uniform_name,
                                      float def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddConstFloat(uniform_name, def_val);
}

void EditorBase::AttachSliderInt(const std::string& name,
                                     const std::string& uniform_name, 
                                     const std::string& ui_name,
                                     int min_val, int max_val, int def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddSliderInt(uniform_name, ui_name, 
                                        min_val, max_val, def_val);
}

void EditorBase::AttachSliderFloat(const std::string& name,
                                       const std::string& uniform_name, 
                                       const std::string& ui_name,
                                       float min_val, float max_val, 
                                       float def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddSliderFloat(uniform_name, ui_name, 
                                          min_val, max_val, def_val);
}

void EditorBase::AttachColorEdit3(const std::string& name,
                                      const std::string& uniform_name, 
                                      const std::string& ui_name,
                                      glm::vec3 def_val) 
{
    if(m_Procedures.count(name))
        m_Procedures[name].AddColorEdit3(uniform_name, ui_name, def_val);
}

void EditorBase::AttachGLEnum(const std::string& name,
                                  const std::string& uniform_name,
                                  const std::string& ui_name,
                                  const std::vector <std::string>& labels)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddGLEnum(uniform_name, ui_name, labels);
}

//===========================================================================

void AddProcedureInstanceImpl(std::unordered_map<std::string, Procedure>& procedures,
                              std::vector<ProcedureInstance>& instances,
                              const std::string& name) {
    if (procedures.count(name)) {
        instances.push_back(ProcedureInstance(name));

        auto& procedure = procedures[name];

        for (auto& task : procedure.m_Tasks) {
            auto type = task->getType();

            switch (type) {
            case TaskType::ConstInt:
            {
                ConstIntTask* typed_task =
                    dynamic_cast<ConstIntTask*>(task.get());

                instances.back().Data.push_back(typed_task->Value);
                break;
            }
            case TaskType::ConstFloat:
            {
                ConstFloatTask* typed_task =
                    dynamic_cast<ConstFloatTask*>(task.get());

                instances.back().Data.push_back(typed_task->Value);
                break;
            }
            case TaskType::SliderInt:
            {
                SliderIntTask* typed_task =
                    dynamic_cast<SliderIntTask*>(task.get());

                instances.back().Data.push_back(typed_task->Def);
                break;
            }
            case TaskType::SliderFloat:
            {
                SliderFloatTask* typed_task =
                    dynamic_cast<SliderFloatTask*>(task.get());

                instances.back().Data.push_back(typed_task->Def);
                break;
            }
            case TaskType::ColorEdit3:
            {
                ColorEdit3Task* typed_task =
                    dynamic_cast<ColorEdit3Task*>(task.get());

                instances.back().Data.push_back(typed_task->Def);
                break;
            }
            case TaskType::GLEnum:
            {
                GLEnumTask* typed_task =
                    dynamic_cast<GLEnumTask*>(task.get());

                instances.back().Data.push_back(0);
                break;
            }
            default:
            {
                break;
            }
            }
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
    : m_Name(name), m_InstanceID(InstanceCount++) {}

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

unsigned int TextureEditor::InstanceCount = 0;

//===========================================================================

TextureArrayEditor::TextureArrayEditor(const std::string& name, int n)
    : m_Name(name), m_InstanceID(InstanceCount++)
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

unsigned int TextureArrayEditor::InstanceCount = 0;