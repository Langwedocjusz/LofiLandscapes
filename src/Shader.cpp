#include "Shader.h"

#include <fstream>
#include <sstream>

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
        std::cerr << "Error: Vertex compilation failed: \n" 
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
        std::cerr << "Error: Fragment compilation failed: \n" 
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

Shader::~Shader() {
    glDeleteProgram(m_ID);
}

void Shader::Bind() {
    glUseProgram(m_ID);
}
