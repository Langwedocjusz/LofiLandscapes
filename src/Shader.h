#pragma once

#include <string>

class Shader{
public:
    Shader(const std::string& vert_path, const std::string& frag_path);
    ~Shader();

    void Bind();
private:
    unsigned int m_ID = 0;
};
