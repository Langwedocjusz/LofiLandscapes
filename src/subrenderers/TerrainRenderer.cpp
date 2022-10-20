#include "TerrainRenderer.h"

#include "glad/glad.h"

bool operator==(const TerrainSettings& lhs, const TerrainSettings& rhs) {
    return (lhs.N == rhs.N) && (lhs.L == rhs.L);
}

bool operator!=(const TerrainSettings& lhs, const TerrainSettings& rhs) {
    return ~(lhs==rhs);
}

void TerrainRenderer::GenerateGrid(std::vector<float>& vert, std::vector<unsigned int>& idx, 
        unsigned int N, float L, float global_offset_x, float global_offset_y, 
        unsigned int LodLevel) 
{
    unsigned int start = static_cast<unsigned int>(vert.size())/4;
    float base_offset = m_Settings.L/float(m_Settings.N-1);

    //Vertex data:
    for (int i=0; i<N*N; i++) {
        float origin[2] = {global_offset_x - L/2.0f, global_offset_y - L/2.0f};
        float offset[2] = {float(i%N)*(L/(N-1)), float(i/N)*(L/(N-1))};

        vert.push_back(origin[0] + offset[0]);
        vert.push_back(0.0f);
        vert.push_back(origin[1] + offset[1]);
        vert.push_back(std::pow(2, LodLevel) * base_offset); //m_L/float(m_N-1));
    }

    //Index data:
    for (int i=0; i<N*N; i++) {
        unsigned int ix = i % N;
        unsigned int iy = i/N;

        if (ix == N-1) continue;
        if (iy == N-1) continue;

        idx.push_back(start+i);
        idx.push_back(start+i+1);
        idx.push_back(start+i+N);
        
        idx.push_back(start+i+1);
        idx.push_back(start+i+1+N);
        idx.push_back(start+i+N);
    }
}

TerrainRenderer::TerrainRenderer() 
    : m_DisplaceShader("res/shaders/displace.glsl")
{
    auto GenBuffers = [](unsigned int&vao, unsigned int&vbo, unsigned int& ebo,
            std::vector<float>& vertex_data,
            std::vector<unsigned int>& index_data){
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), 
                &vertex_data[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                sizeof(unsigned int) * index_data.size(), 
                &index_data[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    };

    //Lod 0
    GenerateGrid(m_TerrainVertexData, m_TerrainIndexData, 
            4*m_Settings.N, 4.0f*m_Settings.L, 0.0f, 0.0f, 0);
    GenBuffers(m_TerrainVAO, m_TerrainVBO, m_TerrainEBO,
               m_TerrainVertexData, m_TerrainIndexData);

    //Lod 1
    float offsets[24] = {-3.0f, 3.0f, -1.0f, 3.0f,  1.0f, 3.0f,  3.0f, 3.0f,
                         -3.0f, 1.0f,  3.0f, 1.0f, -3.0f,-1.0f,  3.0f,-1.0f,
                         -3.0f,-3.0f, -1.0f,-3.0f,  1.0f,-3.0f,  3.0f,-3.0f};

    for (int i=0; i<12; i++){
        GenerateGrid(m_TerrainVertexData2, m_TerrainIndexData2, 
                m_Settings.N, 2.0f*m_Settings.L, 
                m_Settings.L*offsets[2*i], m_Settings.L*offsets[2*i+1],
                1);    
    }

    GenBuffers(m_TerrainVAO2, m_TerrainVBO2, m_TerrainEBO2,
               m_TerrainVertexData2, m_TerrainIndexData2);

    //Lod 2
    for (int i=0; i<12; i++){
        GenerateGrid(m_TerrainVertexData3, m_TerrainIndexData3, 
                m_Settings.N, 4.0f*m_Settings.L, 
                2.0f*m_Settings.L*offsets[2*i], 
                2.0f*m_Settings.L*offsets[2*i+1],
                2);    
    }

    GenBuffers(m_TerrainVAO3, m_TerrainVBO3, m_TerrainEBO3, 
            m_TerrainVertexData3, m_TerrainIndexData3);
    
    //Lod 3
    for (int i=0; i<12; i++){
        GenerateGrid(m_TerrainVertexData4, m_TerrainIndexData4, 
                m_Settings.N, 8.0f*m_Settings.L, 
                4.0f*m_Settings.L*offsets[2*i], 
                4.0f*m_Settings.L*offsets[2*i+1],
                3);    
    }

    GenBuffers(m_TerrainVAO4, m_TerrainVBO4, m_TerrainEBO4, 
            m_TerrainVertexData4, m_TerrainIndexData4);

}

TerrainRenderer::~TerrainRenderer() {

}

void TerrainRenderer::DisplaceVertices(float scale_xz, float scale_y,
                                       float offset_x, float offset_z) {
    m_DisplaceShader.Bind();
    m_DisplaceShader.setUniform2f("uPos", offset_x, offset_z);
    m_DisplaceShader.setUniform1f("uScaleXZ", scale_xz);
    m_DisplaceShader.setUniform1f("uScaleY", scale_y);
    
    //Lod0
    glBindVertexArray(m_TerrainVAO);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TerrainVBO);
    glDispatchCompute(2 * m_TerrainVertexData.size() / 1024, 1, 1);
    
    //Lod 1 
    glBindVertexArray(m_TerrainVAO2);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TerrainVBO2);
    glDispatchCompute(2 * m_TerrainVertexData2.size() / 1024, 1, 1);
    
    //Lod 2 
    glBindVertexArray(m_TerrainVAO3);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TerrainVBO3);
    glDispatchCompute(2 * m_TerrainVertexData3.size() / 1024, 1, 1);

    //Lod 3
    glBindVertexArray(m_TerrainVAO4);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_TerrainVBO4);
    glDispatchCompute(2 * m_TerrainVertexData4.size() / 1024, 1, 1);
}

void TerrainRenderer::BindGeometry() {
}

void TerrainRenderer::Draw() {
    glBindVertexArray(m_TerrainVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO);
    glDrawElements(GL_TRIANGLES, 6*4*m_Settings.N*4*m_Settings.N,
            GL_UNSIGNED_INT, 0);

    glBindVertexArray(m_TerrainVAO2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO2);
    glDrawElements(GL_TRIANGLES, 12*6*m_Settings.N*m_Settings.N,
            GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(m_TerrainVAO3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO3);
    glDrawElements(GL_TRIANGLES, 12*6*m_Settings.N*m_Settings.N,
            GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(m_TerrainVAO4);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TerrainEBO4);
    glDrawElements(GL_TRIANGLES, 12*6*m_Settings.N*m_Settings.N,
            GL_UNSIGNED_INT, 0);
}
