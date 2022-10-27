#include "TerrainRenderer.h"

#include "glad/glad.h"

#include <iostream>

void ClipmapRing::GenerateGrid(
        std::vector<float>& vert, std::vector<unsigned int>& idx, 
        unsigned int N, unsigned int n, float L, float l, 
        float global_offset_x, float global_offset_y, unsigned int LodLevel)  
{
    unsigned int start = static_cast<unsigned int>(vert.size())/4;
    float base_offset = L/float(N-1);

    //Vertex data:
    for (int i=0; i<n*n; i++) {
        float origin[2] = {global_offset_x - l/2.0f, global_offset_y - l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        vert.push_back(origin[0] + offset[0]);
        vert.push_back(0.0f);
        vert.push_back(origin[1] + offset[1]);
        vert.push_back(std::pow(2, LodLevel) * base_offset);
    }

    //Index data:
    for (int i=0; i<n*n; i++) {
        unsigned int ix = i % n;
        unsigned int iy = i/n;

        if (ix == n-1) continue;
        if (iy == n-1) continue;

        idx.push_back(start+i);
        idx.push_back(start+i+1);
        idx.push_back(start+i+n);
        
        idx.push_back(start+i+1);
        idx.push_back(start+i+1+n);
        idx.push_back(start+i+n);
    }
}

ClipmapRing::ClipmapRing(int N, float L, unsigned int level) {
    //Provide Vertes & Index data:
    if (level == 0) {
        GenerateGrid(m_VertexData, m_IndexData, N, 4*N, L, 4.0f*L, 0.0f, 0.0f, 0);
        m_ElementCount = 6*4*N*4*N;
    }

    else {
        const float offsets[24] = 
            {-3.0f, 3.0f, -1.0f, 3.0f,  1.0f, 3.0f,  3.0f, 3.0f,
             -3.0f, 1.0f,  3.0f, 1.0f, -3.0f,-1.0f,  3.0f,-1.0f,
             -3.0f,-3.0f, -1.0f,-3.0f,  1.0f,-3.0f,  3.0f,-3.0f};

        auto pow2 = [](unsigned int x){
            unsigned int res = 1;
            for (int i=0; i<x; i++) {res *= 2;}
            return res;
        };
        
        const float scale = static_cast<float>(pow2(level-1));

        for (int i=0; i<12; i++){
            GenerateGrid(m_VertexData, m_IndexData, N, N, L, 2.0f*scale*L, 
                         scale*L*offsets[2*i], scale*L*offsets[2*i+1], level);    
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

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 
                              (void*)0);
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


TerrainRenderer::TerrainRenderer() 
    : m_DisplaceShader("res/shaders/displace.glsl"),
      m_Lod0(m_N, m_L, 0),
      m_Lod1(m_N, m_L, 1),
      m_Lod2(m_N, m_L, 2),
      m_Lod3(m_N, m_L, 3)
{}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::DisplaceVertices(float scale_xz, float scale_y,
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

void TerrainRenderer::BindAndDraw() {
   m_Lod0.Draw(); 
   m_Lod1.Draw(); 
   m_Lod2.Draw(); 
   m_Lod3.Draw(); 
}
