#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <vector>

class Shader{
public:
    void Bind();
    
    virtual void Reload() = 0;

    void setUniform1i(const std::string& name, int x);
    void setUniform1f(const std::string& name, float x);
    void setUniform2f(const std::string& name, float x, float y);
    void setUniform3f(const std::string& name, glm::vec3 v);
    void setUniform3f(const std::string& name, float x[3]);
    void setUniform4f(const std::string& name, float x[4]);
    void setUniformMatrix4fv(const std::string& name, glm::mat4 mat);
protected:
    unsigned int m_ID = 0;

    unsigned int getUniformLocation(const std::string& name);
    std::vector<std::pair<std::string, unsigned int>> m_UniformCache;
};

class VertFragShader : public Shader {
public:
    VertFragShader(const std::string& vert_path, const std::string& frag_path);
    ~VertFragShader();

    void Reload() override;
private:
    void Build();

    std::string m_VertPath, m_FragPath;
};

class ComputeShader : public Shader {
public:
    ComputeShader(const std::string& compute_path);
    ~ComputeShader();

    void Reload() override;
private:
    void Build();

    std::string m_ComputePath;
};