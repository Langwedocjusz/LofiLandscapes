#include "ClipmapRenderer.h"

#include "glad/glad.h"

#include <iostream>

Drawable::Drawable() {}

Drawable::~Drawable() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Drawable::GenBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VertexData.size(), 
                 &VertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(unsigned int) * IndexData.size(), 
                 &IndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Drawable::DispatchCompute() {
    glBindVertexArray(VAO);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBO);
    glDispatchCompute(2 * VertexData.size() / 1024, 1, 1);
}

void Drawable::Draw() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, 0);
}

void ClipmapRing::GenerateGrid(
        unsigned int N, float L, float global_offset_x, float global_offset_y, 
        unsigned int LodLevel)  
{
    std::vector<float> &verts = m_Grid.VertexData;
    std::vector<unsigned int> &elements = m_Grid.IndexData;

    const unsigned int start = static_cast<unsigned int>(verts.size())/4;
    const float base_offset = L/float(N-1);

    const unsigned int n = (LodLevel==0) ? 4*N - 3 : N; 
    const float scale = std::pow(2.0f, LodLevel-1);
    const float l = (LodLevel==0) ? 4.0f*L : 2.0f*scale*L;

    //Vertex data:
    for (int i=0; i<n*n; i++) {
        float origin[2] = {global_offset_x - l/2.0f, global_offset_y - l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        verts.push_back(origin[0] + offset[0]);
        verts.push_back(0.0f);
        verts.push_back(origin[1] + offset[1]);
        verts.push_back(std::pow(2, LodLevel) * base_offset);
    }

    //Index data:
    for (int i=0; i<n*n; i++) {
        unsigned int ix = i%n;
        unsigned int iy = i/n;

        if (ix == n-1) continue;
        if (iy == n-1) continue;

        elements.push_back(start+i);
        elements.push_back(start+i+1);
        elements.push_back(start+i+n);
        
        elements.push_back(start+i+1);
        elements.push_back(start+i+1+n);
        elements.push_back(start+i+n);
    }
}

void ClipmapRing::GenerateFill(unsigned int N, float L, unsigned int LodLevel) {
    std::vector<float> &verts_x = m_FillX.VertexData;
    std::vector<float> &verts_y = m_FillY.VertexData;
    
    std::vector<unsigned int> &elements_x = m_FillX.IndexData; 
    std::vector<unsigned int> &elements_y = m_FillY.IndexData;
    
    const float base_offset = L/float(N-1);

    const unsigned int n = 4*N-3; 
    const float scale = (LodLevel==0) ? 1.0f : std::pow(2.0f, LodLevel-1);
    const float l = 4.0f*scale*L;

    //Vertex data:
    for (int i=0; i<2*n; i++) {
        float origin[2] = {-l/2.0f, -l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        verts_x.push_back(origin[0] + offset[0]);
        verts_x.push_back(0.0f);
        verts_x.push_back(origin[1] + offset[1]);
        verts_x.push_back(std::pow(2, LodLevel) * base_offset);
        
        verts_y.push_back(origin[1] + offset[1]);
        verts_y.push_back(0.0f);
        verts_y.push_back(origin[0] + offset[0]);
        verts_y.push_back(std::pow(2, LodLevel) * base_offset);
    }

    //Index data:
    for (int i=0; i<n; i++) {
        unsigned int ix = i%n;
        unsigned int iy = i/n;

        if (ix == n-1) continue;
        if (iy == n-1) continue;

        elements_x.push_back(i);
        elements_x.push_back(i+1);
        elements_x.push_back(i+n);
        
        elements_x.push_back(i+1);
        elements_x.push_back(i+1+n);
        elements_x.push_back(i+n);
        
        elements_y.push_back(i+n);
        elements_y.push_back(i+1);
        elements_y.push_back(i);
        
        elements_y.push_back(i+n);
        elements_y.push_back(i+1+n);
        elements_y.push_back(i+1);
    }
}

ClipmapRing::ClipmapRing(int N, float L, unsigned int level) {
    //-----Provide Vertes & Index data:
    //Main grid:
    if (level == 0) {
        GenerateGrid(N, L, 0.0f, 0.0f, level);
        m_Grid.ElementCount = 6*4*N*4*N;
    }

    else {
        const float scale = std::pow(2.0, level-1);
        
        const float offsets[24] = 
            {-3.0f, 3.0f, -1.0f, 3.0f,  1.0f, 3.0f,  3.0f, 3.0f,
             -3.0f, 1.0f,  3.0f, 1.0f, -3.0f,-1.0f,  3.0f,-1.0f,
             -3.0f,-3.0f, -1.0f,-3.0f,  1.0f,-3.0f,  3.0f,-3.0f};

        for (int i=0; i<12; i++){
            GenerateGrid(N, L, scale*L*offsets[2*i], scale*L*offsets[2*i+1],
                         level);    
        }

        m_Grid.ElementCount = 12*6*N*N;
    }

    //Fill meshes
    GenerateFill(N, L, level);
    m_FillX.ElementCount = 6*4*N;
    m_FillY.ElementCount = 6*4*N;

    //-----Generate corresponding GL buffers:
    m_Grid.GenBuffers();
    m_FillX.GenBuffers();
    m_FillY.GenBuffers();
}

ClipmapRing::~ClipmapRing() {}

void ClipmapRing::DispatchCompute() {
    m_Grid.DispatchCompute();
    m_FillX.DispatchCompute();
    m_FillY.DispatchCompute();
}

void ClipmapRing::Draw() {
    m_Grid.Draw();
    m_FillX.Draw();
    m_FillY.Draw();
}


ClipmapRenderer::ClipmapRenderer() 
    : m_DisplaceShader("res/shaders/displace.glsl"),
      m_Lod0(m_N, m_L, 0),
      m_Lod1(m_N, m_L, 1),
      m_Lod2(m_N, m_L, 2),
      m_Lod3(m_N, m_L, 3),
      m_Lod4(m_N, m_L, 4)
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
    m_Lod4.DispatchCompute();
}

void ClipmapRenderer::BindAndDraw() {
   m_Lod0.Draw(); 
   m_Lod1.Draw(); 
   m_Lod2.Draw(); 
   m_Lod3.Draw(); 
   m_Lod4.Draw(); 
}
