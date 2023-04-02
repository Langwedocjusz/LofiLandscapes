#include "Clipmap.h"

#include "glad/glad.h"

#include <algorithm>
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
    const unsigned int invocations = std::max(size_t(1), 2*VertexData.size()/1024);

    glBindVertexArray(VAO);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 
                | GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBO);

    glDispatchCompute(invocations, 1, 1);
}

void Drawable::Draw() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, 0);
}

void GenerateGrid(Drawable& grid,
                  unsigned int N, float L, int LodLevel,
                  float global_offset_x, float global_offset_y
                  )  
{
    //Set elements count
    grid.ElementCount = (LodLevel == 0) ? 6 * 2*N * 2*N : 6 * N * N;

    //Aliases
    auto& verts    = grid.VertexData;
    auto& elements = grid.IndexData;

    //Generate data
    const float base_offset = L / float(N - 1);
    unsigned int lod = std::max(0, LodLevel - 1);

    const unsigned int n = (LodLevel == 0) ? 2*N-1 : N;
    const float scale = std::pow(2.0f, lod);
    const float l = 2.0f * scale * L;

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

        elements.push_back(i);
        elements.push_back(i+1);
        elements.push_back(i+n);
        
        elements.push_back(i+1);
        elements.push_back(i+1+n);
        elements.push_back(i+n);
    }
}

void GenerateFill(Drawable& fill_x, Drawable& fill_y, 
                  unsigned int N, float L, int LodLevel) 
{
    //Set element counts
    fill_x.ElementCount = 6 * 4 * N;
    fill_y.ElementCount = 6 * 4 * N;

    //Aliases
    auto& verts_x    = fill_x.VertexData;
    auto& elements_x = fill_x.IndexData;

    auto& verts_y    = fill_y.VertexData;
    auto& elements_y = fill_y.IndexData;

    //Generate data
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
    const float scale = std::pow(2.0f, std::max(0, int(level) - 1));
    const float l = scale * L;

    //Main grid:
    std::vector<std::pair<float, float>> offsets;

    if (level == 0) {
        offsets = { {-1.0f, 1.0f}, {1.0f, 1.0f},
                    {-1.0f,-1.0f}, {1.0f,-1.0f} };
    }

    else {
        offsets = { {-3.0f, 3.0f}, {-1.0f, 3.0f}, { 1.0f, 3.0f},  {3.0f, 3.0f},
                    {-3.0f, 1.0f}, { 3.0f, 1.0f}, {-3.0f,-1.0f},  {3.0f,-1.0f},
                    {-3.0f,-3.0f}, {-1.0f,-3.0f}, { 1.0f,-3.0f},  {3.0f,-3.0f} };
    }

    for (size_t i = 0; i < offsets.size(); i++) {

        m_Grid.push_back(Drawable());

        m_Bounds.push_back(AABB{
            glm::vec3(l * offsets[i].first, 0.45f, l * offsets[i].second), //center pos
            glm::vec3(l, 0.55f, l)  //extents
        });

        GenerateGrid(
            m_Grid[i], N, L, level,
            l * offsets[i].first, l * offsets[i].second
        );
    }

    //Fill meshes
    GenerateFill(m_FillX, m_FillY, N, L, level);

    //-----Generate corresponding GL buffers:
    for (int i=0; i<m_Grid.size(); i++)
        m_Grid[i].GenBuffers();

    m_FillX.GenBuffers();
    m_FillY.GenBuffers();
}

ClipmapRing::~ClipmapRing() {}

void ClipmapRing::DispatchCompute() {
    for (int i = 0; i < m_Grid.size(); i++)
        m_Grid[i].DispatchCompute();

    m_FillX.DispatchCompute();
    m_FillY.DispatchCompute();
}

void ClipmapRing::Draw(const Camera& cam, float scale_y) {
    for (int i = 0; i < m_Grid.size(); i++) {
        //Frustum culling
        if (cam.IsInFrustum(m_Bounds[i], scale_y)) {
            m_Grid[i].Draw();
            //std::cout << "1";
        }

        //else std::cout << "0";
    }
        
    m_FillX.Draw();
    m_FillY.Draw();
}

Clipmap::Clipmap() 
    : m_DisplaceShader("res/shaders/displace.glsl")
{}

void Clipmap::Init(int subdivisions, int levels) {
    m_BaseOffset = m_L / float(subdivisions);

    m_LodLevels.reserve(levels);

    for (int i = 0; i < levels; i++)
        m_LodLevels.emplace_back(subdivisions + 1, m_L, i);
}

Clipmap::~Clipmap() {}

void Clipmap::DisplaceVertices(float scale_xz, float scale_y,
                                       glm::vec2 pos)
{
    m_DisplaceShader.Bind();
    m_DisplaceShader.setUniform2f("uPos", pos.x, pos.y); //y would actually be z in 3d
    m_DisplaceShader.setUniform1f("uScaleXZ", scale_xz);
    m_DisplaceShader.setUniform1f("uScaleY", scale_y);

    for (int i = 0; i < m_LodLevels.size(); i++)
        m_LodLevels[i].DispatchCompute();
}

void Clipmap::DisplaceVertices(float scale_xz, float scale_y,
                                       glm::vec2 curr, glm::vec2 prev) 
{
    m_DisplaceShader.Bind();
    m_DisplaceShader.setUniform2f("uPos", curr.x, curr.y); //y would actually be z in 3d
    m_DisplaceShader.setUniform1f("uScaleXZ", scale_xz);
    m_DisplaceShader.setUniform1f("uScaleY", scale_y);

    auto update = [curr, prev, this](int level) {
        float scale = std::pow(2, level) * m_BaseOffset;

        glm::vec2 p_offset = prev - glm::mod(prev, scale);
        glm::vec2 c_offset = curr - glm::mod(curr, scale);

        return (p_offset.x != c_offset.x) || (p_offset.y != c_offset.y);
    };

    for (int i = 0; i < m_LodLevels.size(); i++) {
        if (update(i)) m_LodLevels[i].DispatchCompute();
    }
}

void Clipmap::BindAndDraw(const Camera& cam, float scale_y) {

    for (int i = 0; i < m_LodLevels.size(); i++) {
        //std::cout << "Level " << i << ": ";
        m_LodLevels[i].Draw(cam, scale_y);
        //std::cout << '\n';
    }   
}