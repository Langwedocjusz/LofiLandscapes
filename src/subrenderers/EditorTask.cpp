#include "EditorTask.h"

#include "ImGuiUtils.h"

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

    ImGuiUtils::ColSliderInt(UiName, &value, Min, Max, suffix);

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

    ImGuiUtils::ColSliderFloat(UiName, &value, Min, Max, suffix);

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

    ImGuiUtils::ColColorEdit3(UiName, &value, suffix);

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

    ImGuiUtils::ColCombo(UiName, Labels, value, suffix);

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