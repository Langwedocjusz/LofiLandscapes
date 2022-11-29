#pragma once

#include "Shader.h"

#include <variant>
#include <memory>
#include <unordered_map>

typedef std::pair<int, char*> GLEnumData;
typedef std::variant<int, float, glm::vec3, GLEnumData> InstanceData;

enum class TaskType{
    None, ConstInt, ConstFloat, SliderInt, SliderFloat, ColorEdit3, GLEnum
};

class EditorTask{
public:
    virtual TaskType getType() {return TaskType::None;}
};

class ConstIntTask : public EditorTask{
public:
    ConstIntTask(const std::string& uniform_name, int val);
    ~ConstIntTask() = default;

    TaskType getType() override {return TaskType::ConstInt;}

    std::string UniformName;
    const int Value;
};

class ConstFloatTask : public EditorTask{
public:
    ConstFloatTask(const std::string& uniform_name, float val);
    ~ConstFloatTask() = default;

    TaskType getType() override {return TaskType::ConstFloat;}

    std::string UniformName;
    const float Value;
};

class SliderIntTask : public EditorTask {
public:
    SliderIntTask(const std::string& uniform_name,
                  const std::string& ui_name,
                  int min, int max, int def);

    ~SliderIntTask() = default;

    TaskType getType() override {return TaskType::SliderInt;}

    std::string UniformName, UiName;
    int Min, Max, Def;
};

class SliderFloatTask : public EditorTask {
public:
    SliderFloatTask(const std::string& uniform_name,
                    const std::string& ui_name,
                    float min, float max, float def);

    ~SliderFloatTask() = default;

    TaskType getType() override {return TaskType::SliderFloat;}

    std::string UniformName, UiName;
    float Min, Max, Def;
};

class ColorEdit3Task : public EditorTask {
public:
    ColorEdit3Task(const std::string& uniform_name,
                   const std::string& ui_name,
                   glm::vec3 def);
    ~ColorEdit3Task() = default;

    TaskType getType() override {return TaskType::ColorEdit3;}

    std::string UniformName, UiName;
    glm::vec3 Def;
};

class GLEnumTask : public EditorTask {
public:
    GLEnumTask(const std::string& uniform_name,
               const std::string& ui_name,
               const std::vector<std::string>& labels);

    ~GLEnumTask() = default;

    TaskType getType() override { return TaskType::GLEnum; }

    std::string UniformName, UiName;
    std::vector<std::string> m_Labels;
};

class Procedure{
public:
     Procedure() = default;
    ~Procedure() = default;

    void CompileShader(const std::string& filepath);

    void AddConstInt(const std::string& uniform_name, int def_val);
    void AddConstFloat(const std::string& uniform_name, float def_val);

    void AddSliderInt(const std::string& uniform_name, 
                      const std::string& ui_name,
                      int min_val, int max_val, int def_val);
    
    void AddSliderFloat(const std::string& uniform_name, 
                        const std::string& ui_name,
                        float min_val, float max_val, float def_val);

    void AddColorEdit3(const std::string& uniform_name, 
                       const std::string& ui_name,
                       glm::vec3 def_val);

    void AddGLEnum(const std::string& uniform_name,
                   const std::string& ui_name,
                   const std::vector<std::string>& labels);

    void OnDispatch(int res, const std::vector<InstanceData>& data);
    bool OnImGui(std::vector<InstanceData>& data);

    std::unique_ptr<Shader> m_Shader;
    std::vector<std::unique_ptr<EditorTask>> m_Tasks;
};

class ProcedureInstance{
public:
    ProcedureInstance(const std::string& name);
    ~ProcedureInstance() = default;
    
    std::string Name;
    std::vector<InstanceData> Data;

    bool KeepAlive = true;
};

class MaterialEditor{
public:
     MaterialEditor() = default;
    ~MaterialEditor() = default;

    void RegisterShader(const std::string& name, const std::string& filepath);
 
    void AttachConstInt(const std::string& name, 
                        const std::string& uniform_name,
                        int def_val);
    
    void AttachConstFloat(const std::string& name, 
                        const std::string& uniform_name,
                        float def_val);

    void AttachSliderInt(const std::string& name, 
                         const std::string& uniform_name, 
                         const std::string& ui_name,
                         int min_val, int max_val, int def_val);
    
    void AttachSliderFloat(const std::string& name, 
                           const std::string& uniform_name, 
                           const std::string& ui_name,
                           float min_val, float max_val, float def_val);

    void AttachColorEdit3(const std::string& name, 
                          const std::string& uniform_name, 
                          const std::string& ui_name,
                          glm::vec3 def_val);

    void AddProcedureInstance(const std::string& name);

    void AttachGLEnum(const std::string& name,
                      const std::string& uniform_name,
                      const std::string& ui_name,
                      const std::vector<std::string>& labels);

    void OnDispatch(int res);
    bool OnImGui();
private:
    std::unordered_map<std::string, Procedure> m_Procedures;
    std::vector<ProcedureInstance> m_Instances;
};
