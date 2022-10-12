#include "TerrainRenderer.h"

#include "glad/glad.h"

TerrainRenderer::TerrainRenderer(unsigned int N, float L) 
    : m_DisplaceShader("res/shaders/displace.glsl"),
      m_N(N), m_L(L)
{
    //Vertex data:
    for (int i=0; i<N*N; i++) {
        float origin[2] = {-L/2.0f, -L/2.0f};
        float offset[2] = {float(i%N)*(L/N), float(i/N)*(L/N)};

        m_TerrainVertexData.push_back(origin[0] + offset[0]);
        m_TerrainVertexData.push_back(0.0f);
        m_TerrainVertexData.push_back(origin[1] + offset[1]);
        m_TerrainVertexData.push_back(1.0f);
    }

    //Index data:
    for (int i=0; i<N*N; i++) {
        unsigned int ix = i % N;
        unsigned int iy = i/N;

        if (ix == N-1) continue;
        if (iy == N-1) continue;

        m_TerrainIndexData.push_back(i);
        m_TerrainIndexData.push_back(i+1);
        m_TerrainIndexData.push_back(i+N);
        
        m_TerrainIndexData.push_back(i+1);
        m_TerrainIndexData.push_back(i+1+N);
        m_TerrainIndexData.push_back(i+N);
    }

    //GL Buffers:
    glGenVertexArrays(1, &m_TerrainVAO);
    glGenBuffers(1, &m_TerrainVBO);
    glGenBuffers(1, &m_TerrainEBO);

    glBindVertexArray(m_TerrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_TerrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_TerrainVertexData.size(), 
            &m_TerrainVertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            sizeof(unsigned int) * m_TerrainIndexData.size(),
            &m_TerrainIndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

TerrainRenderer::~TerrainRenderer() {

}

void TerrainRenderer::DisplaceVertices() {
    m_DisplaceShader.Bind();
    m_DisplaceShader.setUniform1f("uL", m_L);
    
    glBindVertexArray(m_TerrainVAO);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TerrainVBO);
    glDispatchCompute(2 * m_TerrainVertexData.size() / 1024, 1, 1);
}

void TerrainRenderer::BindGeometry() {
    glBindVertexArray(m_TerrainVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO);
}

void TerrainRenderer::Draw() {
    glDrawElements(GL_TRIANGLES, 6*m_N*m_N, GL_UNSIGNED_INT, 0);
}
