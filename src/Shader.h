#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <string>
#include <vector>
#include <unordered_map>

class Shader{
public:
    void Bind();
    void Reload();

    //Basic uniform setting functions
    void setUniformBool(const std::string& name, bool x);
    void setUniform1i(const std::string& name, int x);
    void setUniform2i(const std::string& name, int x, int y);
    void setUniform3i(const std::string& name, int x, int y, int z);
    void setUniform4i(const std::string& name, int x, int y, int z, int w);
    void setUniform1f(const std::string& name, float x);
    void setUniform2f(const std::string& name, float x, float y);
    void setUniform3f(const std::string& name, float x, float y, float z);
    void setUniform4f(const std::string& name, float x, float y, float z, float w);
    void setUniformMatrix4fv(const std::string& name, float data[16]);

    void setUniformSampler1D(const std::string& name, int x);
    void setUniformSampler2D(const std::string& name, int x);
    void setUniformSampler3D(const std::string& name, int x);
    void setUniformSampler2DArray(const std::string& name, int x);
    void setUniformSamplerCube(const std::string& name, int x);

    //Overrides using glm
    void setUniform2i(const std::string& name, glm::ivec2 v);
    void setUniform3i(const std::string& name, glm::ivec3 v);
    void setUniform4i(const std::string& name, glm::ivec4 v);
    void setUniform2f(const std::string& name, glm::vec2 v);
    void setUniform3f(const std::string& name, glm::vec3 v);
    void setUniform4f(const std::string& name, glm::vec4 v);
    void setUniformMatrix4fv(const std::string& name, glm::mat4 mat);
protected:
    virtual void Build() = 0;
    virtual void LogFilepaths() = 0;

    void RetrieveActiveUniforms(const std::vector<std::string>& source_names);
    int getUniformLocation(const std::string& name, uint32_t type);

    struct ShaderUniformInfo{
        int Location;
        uint32_t Type; //GL Enum
        int Size; //Number of elements for array uniforms
    };

    uint32_t m_ID = 0;
    std::unordered_map<std::string, ShaderUniformInfo> m_UniformCache;
};

class VertFragShader : public Shader {
public:
    VertFragShader(const std::string& vert_path, const std::string& frag_path);
    ~VertFragShader();

private:
    void Build() override;
    void LogFilepaths() override;

    std::string m_VertPath, m_FragPath;
};

class ComputeShader : public Shader {
public:
    ComputeShader(const std::string& compute_path);
    ~ComputeShader();

    //Parameters are total numbers of invocations needed.
    //They will be automatically divided by local group sizes defined in the shader source
    void Dispatch(uint32_t size_x, uint32_t size_y, uint32_t size_z) const;

private:
    void Build() override;
    void LogFilepaths() override;

    void RetrieveLocalSizes(const std::string& source_code);

    std::string m_ComputePath;
    uint32_t m_LocalSizeX = 1, m_LocalSizeY = 1, m_LocalSizeZ = 1;
};