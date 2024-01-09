#pragma once

#include <cstdint>

class Quad{
public:
    Quad();
    ~Quad();

    void Draw();
private:
    uint32_t m_VAO, m_VBO, m_EBO;
    
    float m_VertexData[12] = {-1.0f, 1.0f, 0.5f,
                               1.0f, 1.0f, 0.5f,
                               1.0f,-1.0f, 0.5f,
                              -1.0f,-1.0f, 0.5f };
    
    uint32_t m_IndexData[6] = {0,1,3, 1,2,3};
};