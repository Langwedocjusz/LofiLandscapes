#pragma once

#include "Shader.h"

#include "nlohmann/json.hpp"

#include <variant>

typedef std::variant<int, float, glm::vec3, size_t> InstanceData;

class EditorTask {
public:
    virtual ~EditorTask() = 0;

    virtual void OnDispatch(Shader& shader, const InstanceData& data) = 0;
    virtual void OnImGui(InstanceData& /*data*/, bool& /*state*/, const std::string& /*suffix*/) {}
    virtual void OnSerialize(nlohmann::ordered_json& output, InstanceData data) = 0;

    virtual void ProvideDefaultData(std::vector<InstanceData>& data) = 0;
    virtual void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) = 0;
};

class ConstIntTask : public EditorTask {
public:
    ConstIntTask(const std::string& uniform_name, int val);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

    std::string UniformName;
    const int Value;
};

class ConstFloatTask : public EditorTask {
public:
    ConstFloatTask(const std::string& uniform_name, float val);

    void OnDispatch(Shader& shader, const InstanceData& data) override;
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

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
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

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
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

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
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

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
    void OnSerialize(nlohmann::ordered_json& output, InstanceData data) override;

    void ProvideDefaultData(std::vector<InstanceData>& data) override;
    void ProvideData(std::vector<InstanceData>& data, nlohmann::ordered_json& input) override;

    std::string UniformName, UiName;
    std::vector<std::string> Labels;
};