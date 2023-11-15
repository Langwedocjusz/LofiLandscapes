#include "Clipmap.h"
#include "Profiler.h"

#include "glad/glad.h"

#include <algorithm>
#include <iostream>

Drawable::~Drawable() 
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Drawable::GenGLBuffers()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VertexData.size(), 
                 &VertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * IndexData.size(),
                 &IndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Drawable::BindBufferBase(uint32_t id) const
{
    glBindVertexArray(m_VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, id, m_VBO);
}

void Drawable::Draw() const
{
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, 0);
}

float ScaleFromLodLevel(uint32_t level)
{
    const float scale = (level == 0) ? 1.0f : std::pow(2.0f, level - 1);
    return scale;
}

//N - number of verts per line, L - base side length of the grid
void GenerateGrid(Drawable& grid,
                  unsigned int N, float L, int LodLevel,
                  glm::vec2 global_offset)  
{
    //We are treating zeroth level as 2x2 grid instead of 4x4
    //so the number of vertices needs to be twice as large (-1 is due to overlap)
    const uint32_t n = (LodLevel == 0) ? 2 * N - 1 : N;

    grid.VertexCount = n * n;

    //Corresponding element counts (1 quad = 2 triangles = 6 indices)
    grid.ElementCount = (LodLevel == 0) ? 6 * 2*N * 2*N : 6 * N * N;

    //Aliases
    auto& verts    = grid.VertexData;
    auto& elements = grid.IndexData;

    //Generate data
    const float scale = ScaleFromLodLevel(LodLevel);

    //Side length of the entire grid segment
    const float l = 2.0f * scale * L;
    //Side length of one quad
    const float base_offset = L / float(N - 1);

    //Vertex data:
    for (int i=0; i<n*n; i++)
    {
        float origin[2] = {global_offset.x - l/2.0f, global_offset.y - l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        verts.push_back(origin[0] + offset[0]);
        verts.push_back(0.0f);
        verts.push_back(origin[1] + offset[1]);
        verts.push_back(std::pow(2, LodLevel) * base_offset);
        //verts.push_back(0.0f);
    }

    //Index data:
    for (uint32_t i=0; i<n*n; i++)
    {
        uint32_t ix = i%n;
        uint32_t iy = i/n;

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

enum class Orientation {
    Horizontal, Vertical
};

//N - number of verts per line, L - base side length of the grid
void GenerateFill(Drawable& fill, Orientation orientation, 
                  unsigned int N, float L, int LodLevel) 
{
    //Number of vertices in one line (-3 because of overlap)
    const uint32_t n = 4 * N - 3;

    //We are generating a strip consisting of two lines
    fill.VertexCount = 2 * n;
    fill.ElementCount = 6 * 4 * N;

    //Aliases
    auto& verts   = fill.VertexData;
    auto& elements = fill.IndexData;

    //Generate data
    const float scale = ScaleFromLodLevel(LodLevel);

    const float l = 4.0f * scale * L;
    const float base_offset = L/float(N-1);

    //Vertex data:
    for (uint32_t i=0; i<2*n; i++)
    {
        float origin[2] = {-l/2.0f, -l/2.0f};
        float offset[2] = {float(i%n)*(l/(n-1)), float(i/n)*(l/(n-1))};

        switch (orientation)
        {
            case Orientation::Horizontal:
            {
                verts.push_back(origin[0] + offset[0]);
                verts.push_back(0.0f);
                verts.push_back(origin[1] + offset[1]);
                verts.push_back(std::pow(2, LodLevel) * base_offset);
                
                break;
            }
            case Orientation::Vertical:
            {
                verts.push_back(origin[1] + offset[1]);
                verts.push_back(0.0f);
                verts.push_back(origin[0] + offset[0]);
                verts.push_back(std::pow(2, LodLevel) * base_offset);

                break;
            }
        }
    }

    //Index data:
    for (uint32_t i=0; i<n; i++)
    {
        uint32_t ix = i%n;
        uint32_t iy = i/n;

        if (ix == n-1) continue;
        if (iy == n-1) continue;

        switch (orientation)
        {
            case Orientation::Horizontal:
            {
                elements.push_back(i);
                elements.push_back(i + 1);
                elements.push_back(i + n);

                elements.push_back(i + 1);
                elements.push_back(i + 1 + n);
                elements.push_back(i + n);

                break;
            }
            case Orientation::Vertical:
            {
                elements.push_back(i + n);
                elements.push_back(i + 1);
                elements.push_back(i);

                elements.push_back(i + n);
                elements.push_back(i + 1 + n);
                elements.push_back(i + 1);
            }
        }
    }
}

void Clipmap::Init(uint32_t subdivisions, uint32_t levels)
{
    if (subdivisions == 0 || levels == 0)
        return;

    m_BaseOffset = m_BaseSideLength / float(subdivisions);
    m_VertsPerLine = subdivisions + 1;
    m_Levels = levels;

    const uint32_t num_grids = 4 + 12 * (levels - 1);
    const uint32_t num_fills = 2 * levels;

    m_Grids.reserve(num_grids);
    m_Fills.reserve(num_fills);

    for (uint32_t level = 0; level < levels; level++)
    {
        // Scaled side length
        const float scale = ScaleFromLodLevel(level);
        const float l = scale * m_BaseSideLength;

        //Offsets of particular grids within a level (single ring of the clipmap)
        std::vector<glm::vec2> offsets;

        if (level == 0)
        {
            offsets = { {-1.0f, 1.0f}, {1.0f, 1.0f},
                        {-1.0f,-1.0f}, {1.0f,-1.0f} };
        }

        else
        {
            offsets = { {-3.0f, 3.0f}, {-1.0f, 3.0f}, { 1.0f, 3.0f},  {3.0f, 3.0f},
                        {-3.0f, 1.0f}, { 3.0f, 1.0f}, {-3.0f,-1.0f},  {3.0f,-1.0f},
                        {-3.0f,-3.0f}, {-1.0f,-3.0f}, { 1.0f,-3.0f},  {3.0f,-3.0f} };
        }

        for (size_t i = 0; i < offsets.size(); i++)
        {
            m_Grids.emplace_back();

            GenerateGrid(m_Grids.back(), m_VertsPerLine, m_BaseSideLength, level, l * offsets[i]);

            m_Grids.back().GenGLBuffers();

            //Bounding box parameters
            const float bb_center_height = 0.45f;
            const float bb_vertical_extents = 0.55;

            const glm::vec3 center{ l * offsets[i].x, bb_center_height, l * offsets[i].y };
            const glm::vec3 extents{ l, bb_vertical_extents, l };

            m_Grids.back().BoundingBox.Center = center;
            m_Grids.back().BoundingBox.Extents = extents;
        }

        m_Fills.emplace_back();
        GenerateFill(m_Fills.back(), Orientation::Horizontal, m_VertsPerLine, m_BaseSideLength, level);
        m_Fills.back().GenGLBuffers();

        m_Fills.emplace_back();
        GenerateFill(m_Fills.back(), Orientation::Vertical, m_VertsPerLine, m_BaseSideLength, level);
        m_Fills.back().GenGLBuffers();
    }
}

uint32_t Clipmap::NumGridsPerLevel(uint32_t level)
{
    return (level == 0) ? 4 : 12;
}

uint32_t Clipmap::NumFillsPerLevel(uint32_t level)
{
    return 2;
}

uint32_t Clipmap::MaxGridIDUpTo(uint32_t level)
{
    return (level == 0) ? 0 : 4 + (level-1) * 12;
}

uint32_t Clipmap::MaxFillIDUpTo(uint32_t level)
{
    return (level == 0) ? 0 : level * 2;
}

bool Clipmap::LevelShouldUpdate(uint32_t level, glm::vec2 curr, glm::vec2 prev) const
{
    float scale = std::pow(2, level) * m_BaseOffset;

    glm::vec2 p_offset = prev - glm::mod(prev, scale);
    glm::vec2 c_offset = curr - glm::mod(curr, scale);

    return (p_offset.x != c_offset.x) || (p_offset.y != c_offset.y);
}

void Clipmap::RunCompute(std::shared_ptr<ComputeShader> shader, uint32_t binding)
{
    for (const auto& grid : m_Grids)
    {
        grid.BindBufferBase(1);
        shader->Dispatch(grid.VertexCount, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    }

    for (const auto& fill : m_Fills)
    {
        fill.BindBufferBase(1);
        shader->Dispatch(fill.VertexCount, 1, 1);
        glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    }
}

void Clipmap::RunCompute(std::shared_ptr<ComputeShader> shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev)
{
    uint32_t grid_id = 0, fill_id = 0;

    for (uint32_t level = 0; level < m_Levels; level++)
    {
        if (!LevelShouldUpdate(level, curr, prev))
            continue;

        for (uint32_t i = 0; i < NumGridsPerLevel(level); i++)
        {
            auto& grid = m_Grids[grid_id];

            grid.BindBufferBase(1);
            shader->Dispatch(grid.VertexCount, 1, 1);
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            grid_id++;
        }

        for (uint32_t i = 0; i < NumFillsPerLevel(level); i++)
        {
            auto& fill = m_Fills[fill_id];

            fill.BindBufferBase(1);
            shader->Dispatch(fill.VertexCount, 1, 1);
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            fill_id++;
        }
    }
}