#include "Shader.h"

#include <fstream>
#include <sstream>
#include <algorithm>

#include <iostream>

#include "glad/glad.h"

void getCodeFromFile(const std::string& file_path, std::string& output) {
    std::ifstream file(file_path);

    if (!file)
        throw "file not read successfully: " + file_path;

    std::stringstream stream;

    stream << file.rdbuf();

    output = stream.str();
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

Shader::Shader(const std::string& vert_path, const std::string& frag_path) {
    std::string vert_code, frag_code;

    try {
        getCodeFromFile(vert_path, vert_code);
    }
    catch (std::string error_message) {
        std::cerr << "Vertex Shader error: " << error_message << '\n';
    }

    try {
        getCodeFromFile(frag_path, frag_code);
    }
    catch (std::string error_message) {
        std::cerr << "Fragment Shader error: " << error_message << '\n';
    }

    unsigned int vert_id = 0, frag_id = 0;

    try {
        compileShaderCode(vert_code, vert_id, GL_VERTEX_SHADER);
    }
    catch (std::string error_message) {
        std::cerr << "Vertex Shader compilation failed: \n" << error_message << '\n';
        return;
    }

    try {
        compileShaderCode(frag_code, frag_id, GL_FRAGMENT_SHADER);
    }
    catch (std::string error_message) {
        std::cerr << "Fragment Shader compilation failed: \n" << error_message << '\n';
        return;
    }

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
                  << info_log << '\n';
    }

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}


Shader::Shader(const std::string& compute_path) {
    std::string compute_code;

    try {
        getCodeFromFile(compute_path, compute_code);
    }
    catch (std::string error_message) {
        std::cerr << "Compute Shader error: " << error_message << '\n';
    }

    unsigned int compute_id = 0;

    try {
        compileShaderCode(compute_code, compute_id, GL_COMPUTE_SHADER);
    }
    catch (std::string error_message) {
        std::cerr << "Compute Shader compilation failed: \n" << error_message << '\n';
        return;
    }

    m_ID = glCreateProgram();
    glAttachShader(m_ID, compute_id);
    glLinkProgram(m_ID);

    int success = 0;
    char info_log[512];

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

void Shader::setUniformMatrix4fv(const std::string& name, glm::mat4 mat) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}
