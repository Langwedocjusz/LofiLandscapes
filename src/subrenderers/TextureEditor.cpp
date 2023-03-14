#include "TextureEditor.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

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

                auto value = std::get<GLEnumData>(data[i]).first;
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

bool Procedure::OnImGui(std::vector<InstanceData>& data) {
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

                ImGuiUtils::SliderInt(typed_task->UiName, &value, 
                    typed_task->Min, typed_task->Max);

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

                ImGuiUtils::SliderFloat(typed_task->UiName, &value,
                    typed_task->Min, typed_task->Max);

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

                ImGuiUtils::ColorEdit3(typed_task->UiName, &value);

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

                char* current_item = std::get<GLEnumData>(data[i]).second;
                int* ptr = &std::get<GLEnumData>(data[i]).first;
                int value = *ptr;

                std::string& name = typed_task->UiName;

                ImGui::Text(name.c_str());
                ImGui::SameLine(ImGui::GetWindowWidth() / 3);
                ImGui::PushItemWidth(-ImGui::GetStyle().FramePadding.x);

                if (ImGui::BeginCombo(("##"+name).c_str(), current_item))
                {
                    for (int n = 0; n < typed_task->m_Labels.size(); n++)
                    {
                        bool is_selected = (current_item == typed_task->m_Labels[n].c_str());

                        if (ImGui::Selectable(typed_task->m_Labels[n].c_str(), is_selected)) {
                            current_item = &((typed_task->m_Labels[n])[0]);
                            value = n;
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();

                if (value != *ptr) {
                    *ptr = value;
                    res = true;
                    std::get<GLEnumData>(data[i]).second = current_item;
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

                instances.back().Data.push_back(std::pair<int, char*>(0, &((typed_task->m_Labels[0])[0])));
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
                 std::vector<ProcedureInstance>& instances)
{
    bool res = false;

    for (int i = 0; i < instances.size(); i++) {
        auto& instance = instances[i];
        auto& data = instance.Data;

        ImGui::PushID(i);

        if (ImGui::CollapsingHeader(instance.Name.c_str(), &instance.KeepAlive))
        {
            res = res || procedures[instance.Name].OnImGui(data);
        }

        ImGui::PopID();

        if (!instance.KeepAlive) {
            instances.erase(instances.begin() + i);
            res = true;
        }
    }

    return res;
}

//===========================================================================

void TextureEditor::AddProcedureInstance(const std::string& name) {
    AddProcedureInstanceImpl(m_Procedures, m_Instances, name);
}

void TextureEditor::OnDispatch(int res) {
    OnDispatchImpl(m_Procedures, m_Instances, res);
}

bool TextureEditor::OnImGui() {
    return OnImGuiImpl(m_Procedures, m_Instances);
}

//===========================================================================

TextureArrayEditor::TextureArrayEditor(int n) {
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

    return OnImGuiImpl(m_Procedures, instances);
}