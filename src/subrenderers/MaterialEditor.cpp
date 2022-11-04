#include "MaterialEditor.h"

#include "glad/glad.h"
#include "imgui.h"

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
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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

                ImGui::SliderInt(typed_task->UiName.c_str(), &value, 
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

                ImGui::SliderFloat(typed_task->UiName.c_str(), &value, 
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

                ImGui::ColorEdit3(typed_task->UiName.c_str(), 
                                  glm::value_ptr(value));

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

void MaterialEditor::RegisterShader(const std::string& name, 
                                    const std::string& filepath)
{
    if (m_Procedures.count(name)) return;

    m_Procedures[name].CompileShader(filepath);
}

void MaterialEditor::AttachConstInt(const std::string& name, 
                                    const std::string& uniform_name,
                                    int def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddConstInt(uniform_name, def_val);
}
    
void MaterialEditor::AttachConstFloat(const std::string& name, 
                                      const std::string& uniform_name,
                                      float def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddConstFloat(uniform_name, def_val);
}

void MaterialEditor::AttachSliderInt(const std::string& name, 
                                     const std::string& uniform_name, 
                                     const std::string& ui_name,
                                     int min_val, int max_val, int def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddSliderInt(uniform_name, ui_name, 
                                        min_val, max_val, def_val);
}

void MaterialEditor::AttachSliderFloat(const std::string& name, 
                                       const std::string& uniform_name, 
                                       const std::string& ui_name,
                                       float min_val, float max_val, 
                                       float def_val)
{
    if (m_Procedures.count(name))
        m_Procedures[name].AddSliderFloat(uniform_name, ui_name, 
                                          min_val, max_val, def_val);
}

void MaterialEditor::AttachColorEdit3(const std::string& name, 
                                      const std::string& uniform_name, 
                                      const std::string& ui_name,
                                      glm::vec3 def_val) 
{
    if(m_Procedures.count(name))
        m_Procedures[name].AddColorEdit3(uniform_name, ui_name, def_val);
}

void MaterialEditor::AddProcedureInstance(const std::string& name) {
    if (m_Procedures.count(name)) {
        m_Instances.push_back(ProcedureInstance(name));

        auto& procedure = m_Procedures[name];

        for (auto& task : procedure.m_Tasks) {
            auto type = task->getType();

            switch(type) {
                case TaskType::ConstInt:
                {
                    ConstIntTask* typed_task = 
                        dynamic_cast<ConstIntTask*>(task.get());

                    m_Instances.back().Data.push_back(typed_task->Value);
                    break;
                }
                case TaskType::ConstFloat:
                {
                    ConstFloatTask* typed_task = 
                        dynamic_cast<ConstFloatTask*>(task.get());

                    m_Instances.back().Data.push_back(typed_task->Value);
                    break;
                }
                case TaskType::SliderInt:
                {
                    SliderIntTask* typed_task = 
                        dynamic_cast<SliderIntTask*>(task.get());

                    m_Instances.back().Data.push_back(typed_task->Def);
                    break;
                }
                case TaskType::SliderFloat:
                {
                    SliderFloatTask* typed_task = 
                        dynamic_cast<SliderFloatTask*>(task.get());

                    m_Instances.back().Data.push_back(typed_task->Def);
                    break;
                }
                case TaskType::ColorEdit3:
                {
                    ColorEdit3Task* typed_task = 
                        dynamic_cast<ColorEdit3Task*>(task.get());

                    m_Instances.back().Data.push_back(typed_task->Def);
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

void MaterialEditor::OnDispatch(int res) {
    for (auto& instance : m_Instances) {
        auto& data = instance.Data;
        
        m_Procedures[instance.Name].OnDispatch(res, data);
    }
}

bool MaterialEditor::OnImGui() {
    bool res = false;

    int id = 0;

    for (auto& instance : m_Instances) {
        auto& data = instance.Data;

        ImGui::Text(instance.Name.c_str());
        ImGui::PushID(id);
        res = m_Procedures[instance.Name].OnImGui(data);
        ImGui::PopID();
        ImGui::Separator();

        ++id;
    }

    return res;
}
