#pragma once

#include "Shader.h"

#include <fstream>
#include <variant>
#include <memory>
#include <unordered_map>

#include "nlohmann/json.hpp"

typedef std::variant<int, float, glm::vec3> InstanceData;

class EditorTask{
public:

    virtual void OnDispatch(Shader& shader, const InstanceData& data) = 0;
    virtual void OnImGui(InstanceData& data, bool& state, const std::string& suffix) {}
    virtual void OnSerialize(nlohmann::json& output, InstanceData data) = 0;

    virtual void ProvideDefaultData(std::vector<InstanceData>& data) = 0;
};

class ConstIntTask : public EditorTask{
public:
    ConstIntTask(const std::string& uniform_name, int val);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName;
    const int Value;
};

class ConstFloatTask : public EditorTask{
public:
    ConstFloatTask(const std::string& uniform_name, float val);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName;
    const float Value;
};

class SliderIntTask : public EditorTask {
public:
    SliderIntTask(const std::string& uniform_name,
                  const std::string& ui_name,
                  int min, int max, int def);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnImGui(InstanceData& data, bool& state, const std::string& suffix) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName, UiName;
    int Min, Max, Def;
};

class SliderFloatTask : public EditorTask {
public:
    SliderFloatTask(const std::string& uniform_name,
                    const std::string& ui_name,
                    float min, float max, float def);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnImGui(InstanceData& data, bool& state, const std::string& suffix) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName, UiName;
    float Min, Max, Def;
};

class ColorEdit3Task : public EditorTask {
public:
    ColorEdit3Task(const std::string& uniform_name,
                   const std::string& ui_name,
                   glm::vec3 def);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnImGui(InstanceData& data, bool& state, const std::string& suffix) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName, UiName;
    glm::vec3 Def;
};

class GLEnumTask : public EditorTask {
public:
    GLEnumTask(const std::string& uniform_name,
               const std::string& ui_name,
               const std::vector<std::string>& labels);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnImGui(InstanceData& data, bool& state, const std::string& suffix) override;
    void OnSerialize(nlohmann::json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;

    std::string UniformName, UiName;
    std::vector<std::string> Labels;
};

class Procedure{
public:
     Procedure() = default;

    void CompileShader(const std::string& filepath);

    template<class T, typename ... Args>
    void Add(Args ... args) {
        static_assert(std::is_base_of<EditorTask, T>::value, "Add template argument not derived from EditorTask");

        m_Tasks.push_back(std::unique_ptr<EditorTask>(
            new T(args...)
        ));
    }

    void OnDispatch(int res, const std::vector<InstanceData>& data);
    bool OnImGui(std::vector<InstanceData>& data, unsigned int id);

    std::unique_ptr<Shader> m_Shader;
    std::vector<std::unique_ptr<EditorTask>> m_Tasks;
};

class ProcedureInstance{
public:
    ProcedureInstance(const std::string& name);
    
    std::string Name;
    std::vector<InstanceData> Data;

    bool KeepAlive = true;
};

class EditorBase {
public:
    EditorBase() = default;

    void RegisterShader(const std::string& name, const std::string& filepath);

    template<class T, typename ... Args>
    void Attach(const std::string& name, Args ... args)
    {
        static_assert(std::is_base_of<EditorTask, T>::value, "Attach template argument not derived from EditorTask");

        if (m_Procedures.count(name))
            m_Procedures[name].Add<T>(args...);
    }

protected:
    std::unordered_map<std::string, Procedure> m_Procedures;
};

class TextureEditor : public EditorBase{
public:
     TextureEditor(const std::string& name);

    void AddProcedureInstance(const std::string& name);

    void OnDispatch(int res);
    bool OnImGui();

    void OnSerialize(nlohmann::json& output);
private:
    std::vector<ProcedureInstance> m_Instances;
    
    std::string m_Name;

    bool m_PopupOpen = true;

    unsigned int m_InstanceID;
    static unsigned int s_InstanceCount;
};

class TextureArrayEditor : public EditorBase {
public:
    TextureArrayEditor(const std::string& name, int n);

    void AddProcedureInstance(int layer, const std::string& name);

    void OnDispatch(int layer, int res);
    bool OnImGui(int layer);
    void OnSerialize(nlohmann::json& output);
private:
    std::vector<std::vector<ProcedureInstance>> m_InstanceLists;

    std::string m_Name;

    bool m_PopupOpen = true;

    unsigned int m_InstanceID;
    static unsigned int s_InstanceCount;
};
