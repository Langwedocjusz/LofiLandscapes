#include "ClipmapRenderer.h"

#include "glad/glad.h"

#include <iostream>

void ClipmapRing::GenerateGrid(
        unsigned int N, float L, float global_offset_x, float global_offset_y, 
        unsigned int LodLevel)  
{
    const unsigned int start = static_cast<unsigned int>(m_VertexData.size())/4;
    const float base_offset = L/float(N-1);

    const unsigned int n = (LodLevel==0) ? 4*N : N; 
    const float scale = std::pow(2.0f, LodLevel-1);
    const float l = (LodLevel==0) ? 4.0f*L : 2.0f*scale*L;

    //Vertex data:
    for (int i=0; i<n*n; i++) {
        float origin[2] = {global_offset_x - l/2.0f, global_offset_y - l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        m_VertexData.push_back(origin[0] + offset[0]);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(origin[1] + offset[1]);
        m_VertexData.push_back(std::pow(2, LodLevel) * base_offset);
    }

    //Index data:
    for (int i=0; i<n*n; i++) {
        unsigned int ix = i%n;
        unsigned int iy = i/n;

        if (ix == n-1) continue;
        if (iy == n-1) continue;

        m_IndexData.push_back(start+i);
        m_IndexData.push_back(start+i+1);
        m_IndexData.push_back(start+i+n);
        
        m_IndexData.push_back(start+i+1);
        m_IndexData.push_back(start+i+1+n);
        m_IndexData.push_back(start+i+n);
    }
}

ClipmapRing::ClipmapRing(int N, float L, unsigned int level) {
    //Provide Vertes & Index data:
    if (level == 0) {
        GenerateGrid(N, L, 0.0f, 0.0f, 0);
        m_ElementCount = 6*4*N*4*N;
    }

    else {
        const float scale = std::pow(2.0, level-1);
        
        const float offsets[24] = 
            {-3.0f, 3.0f, -1.0f, 3.0f,  1.0f, 3.0f,  3.0f, 3.0f,
             -3.0f, 1.0f,  3.0f, 1.0f, -3.0f,-1.0f,  3.0f,-1.0f,
             -3.0f,-3.0f, -1.0f,-3.0f,  1.0f,-3.0f,  3.0f,-3.0f};

        for (int i=0; i<12; i++){
            GenerateGrid(N, L, scale*L*offsets[2*i], 
                         scale*L*offsets[2*i+1], level);    
        }

        m_ElementCount = 12*6*N*N;
    }
    
    //Generate GL Buffers
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_VertexData.size(), 
                 &m_VertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(unsigned int) * m_IndexData.size(), 
                 &m_IndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

ClipmapRing::~ClipmapRing() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void ClipmapRing::DispatchCompute() {
    glBindVertexArray(m_VAO);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_VBO);
    glDispatchCompute(2 * m_VertexData.size() / 1024, 1, 1);
}

void ClipmapRing::Draw() {
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, m_ElementCount, GL_UNSIGNED_INT, 0);
}


ClipmapRenderer::ClipmapRenderer() 
    : m_DisplaceShader("res/shaders/displace.glsl"),
      m_Lod0(m_N, m_L, 0),
      m_Lod1(m_N, m_L, 1),
      m_Lod2(m_N, m_L, 2),
      m_Lod3(m_N, m_L, 3)
{}

ClipmapRenderer::~ClipmapRenderer() {}

void ClipmapRenderer::DisplaceVertices(float scale_xz, float scale_y,
                                       float offset_x, float offset_z) {
    m_DisplaceShader.Bind();
    m_DisplaceShader.setUniform2f("uPos", offset_x, offset_z);
    m_DisplaceShader.setUniform1f("uScaleXZ", scale_xz);
    m_DisplaceShader.setUniform1f("uScaleY", scale_y);

    m_Lod0.DispatchCompute();
    m_Lod1.DispatchCompute();
    m_Lod2.DispatchCompute();
    m_Lod3.DispatchCompute();
}

void ClipmapRenderer::BindAndDraw() {
   m_Lod0.Draw(); 
   m_Lod1.Draw(); 
   m_Lod2.Draw(); 
   m_Lod3.Draw(); 
}
