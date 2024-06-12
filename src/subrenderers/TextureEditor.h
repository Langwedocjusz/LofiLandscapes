#pragma once

#include "ResourceManager.h"
#include "EditorTask.h"

#include <memory>
#include <unordered_map>

class Procedure {
public:
    Procedure(ResourceManager& manager);

    void CompileShader(const std::string& filepath);

    template<class T, typename ... Args>
    void Add(Args ... args) {
        static_assert(
            std::is_base_of<EditorTask, T>::value,
            "Add template argument not derived from EditorTask"
            );

        m_Tasks.push_back(std::make_unique<T>(args...));
    }

    void OnDispatch(int res, const std::vector<InstanceData>& v_data);
    bool OnImGui(std::vector<InstanceData>& v_data, uint32_t id);

    std::shared_ptr<ComputeShader> m_Shader;
    std::vector<std::unique_ptr<EditorTask>> m_Tasks;

    ResourceManager& m_ResourceManager;
};

class ProcedureInstance {
public:
    ProcedureInstance(const std::string& name);

    std::string Name;
    std::vector<InstanceData> Data;

    bool KeepAlive = true;
};

class EditorBase {
public:
    EditorBase(ResourceManager& manager);

    void RegisterShader(const std::string& name, const std::string& filepath);

    template<class T, typename ... Args>
    void Attach(const std::string& name, Args ... args)
    {
        static_assert(
            std::is_base_of<EditorTask, T>::value,
            "Attach template argument not derived from EditorTask"
            );

        if (m_Procedures.count(name))
            m_Procedures.at(name).Add<T>(args...);
    }

protected:
    std::unordered_map<std::string, Procedure> m_Procedures;
    std::vector<std::string> m_ProcedureNames;

    ResourceManager& m_ResourceManager;
};

class TextureEditor : public EditorBase {
public:
    TextureEditor(ResourceManager& manager, const std::string& name);

    void AddProcedureInstance(const std::string& name);

    void OnDispatch(int res);
    bool OnImGui();

    void OnSerialize(nlohmann::ordered_json& output);
    void OnDeserialize(nlohmann::ordered_json& input);

    std::string getName() const { return m_Name; }
private:
    void AddProcedureInstance(const std::string& name, nlohmann::ordered_json& input);

    std::vector<ProcedureInstance> m_Instances;

    std::string m_Name;

    bool m_PopupOpen = true;

    uint32_t m_InstanceID;
    static uint32_t s_InstanceCount;
};

class TextureArrayEditor : public EditorBase {
public:
    TextureArrayEditor(ResourceManager& manager, const std::string& name, int n);

    void AddProcedureInstance(size_t layer, const std::string& name);

    void OnDispatch(int layer, int res);
    bool OnImGui(int layer);
    void OnSerialize(nlohmann::ordered_json& output);
    void OnDeserialize(nlohmann::ordered_json& input);

    std::string getName() const { return m_Name; }
private:
    void AddProcedureInstance(size_t layer, const std::string& name, nlohmann::ordered_json& input);

    std::vector<std::vector<ProcedureInstance>> m_InstanceLists;

    std::string m_Name;

    bool m_PopupOpen = true;

    uint32_t m_InstanceID;
    static uint32_t s_InstanceCount;
};
