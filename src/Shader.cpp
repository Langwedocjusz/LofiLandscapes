#include "Shader.h"

#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>

#include "glad/glad.h"

Shader::Shader(const std::string& vert_path, const std::string& frag_path) {
    std::string vert_code, frag_code;

    std::ifstream vert_file(vert_path);
    std::ifstream frag_file(frag_path);

    if (!vert_file) 
        std::cerr << "Error: Vertex Shader file not read successfully: " 
                  << vert_path << '\n';

    if(!frag_file)
        std::cerr << "Error: Fragment Shader file not read successfully: " 
                  << frag_path << '\n';

    std::stringstream vert_stream, frag_stream;
    vert_stream << vert_file.rdbuf();
    frag_stream << frag_file.rdbuf();

    vert_code = vert_stream.str();
    frag_code = frag_stream.str();

    const char* vert_code_c = vert_code.c_str();
    const char* frag_code_c = frag_code.c_str();

    unsigned int vert_id=0, frag_id=0;
    int success;
    char info_log[512];

    vert_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_id, 1, &vert_code_c, NULL);
    glCompileShader(vert_id);

    glGetShaderiv(vert_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vert_id, 512, NULL, info_log);
        std::cerr << "Error: Vertex compilation failed (" << vert_path << "): \n" 
                  << info_log << '\n';
        glDeleteShader(vert_id);
        return;
    }

    frag_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_id, 1, &frag_code_c, NULL);
    glCompileShader(frag_id);

    glGetShaderiv(frag_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(frag_id, 512, NULL, info_log);
        std::cerr << "Error: Fragment compilation failed (" << frag_path << "): \n" 
                  << info_log << '\n';
        glDeleteShader(vert_id);
        glDeleteShader(frag_id);
        return;
    }

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vert_id);
    glAttachShader(m_ID, frag_id);
    glLinkProgram(m_ID);

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n" 
                  << info_log << '\n';
    }

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}


Shader::Shader(const std::string& compute_path) {
    std::string compute_code;

    std::ifstream compute_file(compute_path);

    if(!compute_file)
        std::cerr << "Error: Compute Shader file not read successfully: " 
                  << compute_path << '\n';

    std::stringstream compute_stream;
    compute_stream << compute_file.rdbuf();

    compute_code = compute_stream.str();

    const char* compute_code_c = compute_code.c_str();

    unsigned int compute_id=0;
    int success;
    char info_log[512];

    compute_id = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute_id, 1, &compute_code_c, NULL);
    glCompileShader(compute_id);

    glGetShaderiv(compute_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(compute_id, 512, NULL, info_log);
        std::cerr << "Error: Compute shader compilation failed (" 
                  << compute_path << "): \n" << info_log << '\n';
        glDeleteShader(compute_id);
        return;
    }

    m_ID = glCreateProgram();
    glAttachShader(m_ID, compute_id);
    glLinkProgram(m_ID);

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n" 
                  << info_log << '\n';
    }

    glDeleteShader(compute_id);
}

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

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

void Shader::setUniform1i(const std::string& name, int x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1i(location, x);
}

void Shader::setUniform1f(const std::string& name, float x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1f(location, x);
}

void Shader::setUniform2f(const std::string& name, float x, float y){
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2f(location, x, y);
}

void Shader::setUniformMatrix4fv(const std::string& name, glm::mat4 mat) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}
