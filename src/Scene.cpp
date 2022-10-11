#include "Scene.h"

#include "glad/glad.h"

Scene::Scene(int N, float L) 
    : m_N(N)
{
    //Terrain
    //Vertex data:
    for (int i=0; i<N*N; i++) {
        float origin[2] = {-L/2.0f, -L/2.0f};
        float offset[2] = {float(i%N)*(L/N), float(i/N)*(L/N)};

        m_TerrainVertexData.push_back(origin[0] + offset[0]);
        m_TerrainVertexData.push_back(0.0f);
        m_TerrainVertexData.push_back(origin[1] + offset[1]);
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Quad (GL Buffers only):
    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glGenBuffers(1, &m_QuadEBO);

    glBindVertexArray(m_QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_QuadVertexData), &m_QuadVertexData, 
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_QuadIndexData), 
                    &m_QuadIndexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Scene::~Scene() {

}

void Scene::BindTerrain() {
    glBindVertexArray(m_TerrainVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO);
}

void Scene::DrawTerrain() {
    glDrawElements(GL_TRIANGLES, 6*m_N*m_N, GL_UNSIGNED_INT, 0);
}

void Scene::BindQuad() {
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
}

void Scene::DrawQuad() {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
