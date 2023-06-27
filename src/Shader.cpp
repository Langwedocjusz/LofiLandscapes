#include "Shader.h"

#include "glad/glad.h"
#include "Shadinclude.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>

void Shader::Bind() {
    glUseProgram(m_ID);
}

unsigned int Shader::getUniformLocation(const std::string& name) {
    auto search_res = std::find_if(
        m_UniformCache.begin(), m_UniformCache.end(),
        [&name](const std::pair<std::string, unsigned int> element){
            return element.first == name;
    });

    if (search_res != m_UniformCache.end())
        return search_res->second;

    const unsigned int location = glGetUniformLocation(m_ID, name.c_str());
    m_UniformCache.push_back(std::make_pair(name, location));
    return location;
}

//Program type is assumed to be gl enum: {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}
void compileShaderCode(const std::string& source, unsigned int& id, int program_type) {
    const char* source_c = source.c_str();

    int success = 0;
    char info_log[512];

    id = glCreateShader(program_type);

    glShaderSource(id, 1, &source_c, NULL);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(id, 512, NULL, info_log);
        glDeleteShader(id);

        throw std::string(info_log);
    }
}

void VertFragShader::Build() {
    //Get source
    std::string vert_code = Shadinclude::load(m_VertPath, "#include");
    std::string frag_code = Shadinclude::load(m_FragPath, "#include");

    //Compile shaders
    unsigned int vert_id = 0, frag_id = 0;

    try {compileShaderCode(vert_code, vert_id, GL_VERTEX_SHADER);}

    catch (std::string error_message) {
        std::cerr << "Vertex Shader compilation failed: \n"
            << "filepath: " << m_VertPath << '\n'
            << error_message << '\n';
        return;
    }

    try {compileShaderCode(frag_code, frag_id, GL_FRAGMENT_SHADER);}

    catch (std::string error_message) {
        std::cerr << "Fragment Shader compilation failed: \n"
            << "filepath: " << m_FragPath << '\n'
            << error_message << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();

    glAttachShader(m_ID, vert_id);
    glAttachShader(m_ID, frag_id);
    glLinkProgram(m_ID);

    int success = 0;
    char info_log[512];

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);

        std::cerr << "Error: Shader program linking failed: \n"
                  << "filepaths: " << m_VertPath << ", " << m_FragPath << '\n'
                  << info_log << '\n';
    }

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}

VertFragShader::VertFragShader(const std::string& vert_path, const std::string& frag_path) 
    : m_VertPath(vert_path), m_FragPath(frag_path)
{
    Build();
}

void VertFragShader::Reload() {
    if(m_ID != 0) glDeleteProgram(m_ID);

    m_ID = 0;

    Build();
}

VertFragShader::~VertFragShader() {
    glDeleteProgram(m_ID);
}

void ComputeShader::Build() {
    //Get source
    std::string compute_code = Shadinclude::load(m_ComputePath, "#include");

    //Compile shader
    unsigned int compute_id = 0;

    try {compileShaderCode(compute_code, compute_id, GL_COMPUTE_SHADER);}

    catch (std::string error_message) {
        std::cerr << "Compute Shader compilation failed: \n"
            << "filepath: " << m_ComputePath << '\n'
            << error_message << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, compute_id);
    glLinkProgram(m_ID);

    int success = 0;
    char info_log[512];

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n"
                  << "filepath: " << m_ComputePath << '\n'
                  << info_log << '\n';
    }

    glDeleteShader(compute_id);
}

ComputeShader::ComputeShader(const std::string& compute_path) 
    : m_ComputePath(compute_path)
{
    Build();
}

ComputeShader::~ComputeShader() {
    glDeleteProgram(m_ID);
}

void ComputeShader::Reload() {
    glDeleteProgram(m_ID);

    Build();
}

//=====Uniform setting==================================================

void Shader::setUniform1i(const std::string& name, int x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1i(location, x);
}

void Shader::setUniform1f(const std::string& name, float x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1f(location, x);
}

void Shader::setUniform2f(const std::string& name, float x, float y) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2f(location, x, y);
}

void Shader::setUniform3f(const std::string& name, glm::vec3 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3f(location, v.x, v.y, v.z);
}

void Shader::setUniform3f(const std::string& name, float x[3]) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3f(location, x[0], x[1], x[2]);
}

void Shader::setUniform4f(const std::string& name, float x[4]) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4f(location, x[0], x[1], x[2], x[3]);
}

void Shader::setUniform4f(const std::string& name, glm::vec4 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void Shader::setUniformMatrix4fv(const std::string& name, glm::mat4 mat) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}