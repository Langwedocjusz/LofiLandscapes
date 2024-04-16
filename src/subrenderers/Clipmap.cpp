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
    glBufferData(GL_ARRAY_BUFFER, sizeof(ClipmapVertex) * VertexData.size(),
                 &VertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * IndexData.size(),
                 &IndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ClipmapVertex), (void*)(offsetof(ClipmapVertex, PosX)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_INT, GL_FALSE, sizeof(ClipmapVertex), (void*)(offsetof(ClipmapVertex, EdgeFlag)));
    glEnableVertexAttribArray(1);
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
    glDrawElements(GL_TRIANGLES, ElementCount(), GL_UNSIGNED_INT, 0);
}

enum EdgeFlag {
    None   = 0,
    Left   = (1 << 0),
    Right  = (1 << 1),
    Top    = (1 << 2),
    Bottom = (1 << 3),
    LeftBot  = Left | Bottom, 
    LeftTop  = Left | Top, 
    RightBot = Right | Bottom, 
    RightTop = Right | Top
};

//We will only store information about a vertex being on 
//vertical/horizontal edge, or not being at an edge at all.
//This doesn't descrive corners well, but that's not a problem,
//since by construction of the clipmap, the corners are always aligned
//with the higher levels by default.

enum EdgeData {
    NoEdge = 0, Horizontal = 1, Vertical = 2
};

//n - number of verts per line, l - side length of the grid
static void GenerateGrid(Drawable& grid,
                  uint32_t n, float l,
                  glm::vec2 global_offset,
                  EdgeFlag edge_flag)
{
    const auto vert_count = n * n;
    const auto idx_count = 6 * (n - 1) * (n - 1); //1 quad = 6 indices

    const float quad_size = l / static_cast<float>(n - 1);

    //Aliases
    auto& verts = grid.VertexData;
    auto& elements = grid.IndexData;

    verts.reserve(vert_count);
    elements.reserve(idx_count);

    auto GetOffset = [n, quad_size](uint32_t i)
    {
        return glm::vec2{
            quad_size * static_cast<float>(i % n),
            quad_size * static_cast<float>(i / n)
        };
    };

    auto GetEdgeData = [edge_flag, n](uint32_t i) -> int
    {
        const auto idx = i % n;
        const auto idy = i / n;

        const bool left_edge = (idx == 0);
        const bool right_edge = (idx == n - 1);
        const bool bottom_edge = (idy == 0);
        const bool top_edge = (idy == n - 1);

        if (left_edge && ((edge_flag & Left) != None))
            return Vertical;

        if (right_edge && ((edge_flag & Right) != None))
            return Vertical;

        if (top_edge && ((edge_flag & Top) != None))
            return Horizontal;

        if (bottom_edge && ((edge_flag & Bottom) != None))
            return Horizontal;

        return NoEdge;
    };

    //Vertex data:
    for (uint32_t i=0; i<n*n; i++)
    {
        const glm::vec2 origin = glm::vec2(-l / 2.0f) + global_offset;
        const glm::vec2 offset = GetOffset(i);

        verts.push_back(ClipmapVertex{
            origin.x + offset.x, 0.0f, origin.y + offset.y, //position
            GetEdgeData(i)                                  //edge flag
        });
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

//n - number of verts in line
static void GenerateStrip(Drawable& fill, Orientation orientation, 
                  uint32_t n, float quad_size,
                  glm::vec2 global_offset,
                  bool is_trim,
                  bool level_zero)
{
    const auto start_id = fill.VertexCount();

    //Aliases
    auto& verts   = fill.VertexData;
    auto& elements = fill.IndexData;

    auto GenOffset = [n, quad_size, orientation](uint32_t i)
    {
        const glm::vec2 res{
            quad_size* static_cast<float>(i/2),
            quad_size * static_cast<float>(i%2)
        };

        if (orientation == Orientation::Vertical)
            return glm::vec2{ res.y, res.x };

        else
            return res;
    };

    auto GetEdgeData = [orientation, is_trim, level_zero, global_offset, n](uint32_t i) -> int
    {
        bool first_two = (i == 0 || i == 1);
        bool last_two = (i == 2 * n - 1 || i == 2 * n - 2);

        bool horizontal = (orientation == Orientation::Horizontal);

        auto orientation_aligned  = horizontal ? Horizontal : Vertical;
        auto orientation_opposite = horizontal ? Vertical : Horizontal;

        if (is_trim)
        {
            if (first_two || last_two)
                return orientation_opposite;

            else
                return orientation_aligned;
        }

        else
        {
            //On levels higher than zero edge will correspond to either the first two
            //or the last two vertices. 
            bool edge_at_start = (horizontal && (global_offset.x < 0.0f))
                || ((!horizontal) && (global_offset.y < 0.0f));
            

            //On level zero edge corresponds to both start and end.
            if (level_zero)
            {
                if (first_two || last_two)
                    return orientation_opposite;
            }

            else
            {
                if (edge_at_start && first_two)
                    return orientation_opposite;

                else if (!edge_at_start && last_two)
                    return orientation_opposite;
            }
        }

        return NoEdge;
    };

    //Vertex data:
    for (uint32_t i=0; i<2*n; i++)
    {
        const glm::vec2 origin = global_offset;
        const glm::vec2 offset = GenOffset(i);

        verts.push_back(ClipmapVertex{
            origin.x + offset.x, 0.0f, origin.y + offset.y, //position
            GetEdgeData(i)                                  //edge flag
        });
    }

    //Index data:
    for (uint32_t i=0; i<n-1; i++)
    {
        switch (orientation)
        {
            case Orientation::Horizontal:
            {
                elements.push_back(start_id + 2 * i);
                elements.push_back(start_id + 2 * i + 2);
                elements.push_back(start_id + 2 * i + 1);
                                    
                elements.push_back(start_id + 2 * i + 1);
                elements.push_back(start_id + 2 * i + 2);
                elements.push_back(start_id + 2 * i + 3);

                break;
            }
            case Orientation::Vertical:
            {
                elements.push_back(start_id + 2*i);
                elements.push_back(start_id + 2*i + 1);
                elements.push_back(start_id + 2*i + 2);
                                   
                elements.push_back(start_id + 2*i + 1);
                elements.push_back(start_id + 2*i + 3);
                elements.push_back(start_id + 2*i + 2);

                break;
            }
        }
    }
}

Clipmap::~Clipmap()
{
    //glDeleteBuffers(1, &m_SSBO);
    glDeleteBuffers(1, &m_UBO);
}

void Clipmap::Init(uint32_t subdivisions, uint32_t levels)
{
    if (subdivisions == 0 || levels == 0)
        return;

    m_BaseQuadSize = m_BaseGridSize / static_cast<float>(subdivisions);
    m_VertsPerLine = subdivisions + 1;
    m_Levels = levels;

    const uint32_t num_grids = 4 + 12 * (levels - 1);
    const uint32_t num_trims = levels;
    const uint32_t num_fills = levels;

    m_Grids.reserve(num_grids);
    m_Trims.reserve(num_trims);
    m_Fills.reserve(num_fills);

    auto getGridSize = [this](uint32_t lvl){
        //We are treating zeroth level as 2x2 grid instead of 4x4
        //which slightly complicates the computation of grid_size:
        const float scale = (lvl == 0) ? 2.0f : std::pow(2.0f, lvl);

        return scale * m_BaseGridSize;
    };

    auto getQuadSize = [this](uint32_t lvl){
        return std::pow(2, lvl) * m_BaseQuadSize;
    };

    int drawable_id = 0;

    //Will be used for UBO data:
    std::vector<float> quad_sizes;
    std::vector<float> trim_flags;

    //Order here is important, since drawable id is incremented inside each loop
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateGrids(lvl, getGridSize(lvl), getQuadSize(lvl), drawable_id);
        
        for (int i=0; i<NumGridsPerLevel(lvl); i++)
        {
            quad_sizes.push_back(getQuadSize(lvl));
            trim_flags.push_back(0.0f);
        }
    }
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateFills(lvl, getGridSize(lvl), getQuadSize(lvl), drawable_id);
        quad_sizes.push_back(getQuadSize(lvl));
        trim_flags.push_back(0.0f);
    }
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateTrims(lvl, getGridSize(lvl), getQuadSize(lvl), drawable_id);
        quad_sizes.push_back(getQuadSize(lvl));
        trim_flags.push_back(1.0f);
    }

    //Setup the UBO
    std::vector<float> ubo_data;

    //Store only quad sizes
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        ubo_data.push_back(getQuadSize(lvl));
        //Padding needed since ubo's can only use std140 layout
        //Where arrays have 16 byte alignment
        ubo_data.push_back(0.0f);
        ubo_data.push_back(0.0f);
        ubo_data.push_back(0.0f);
    }

    glGenBuffers(1, &m_UBO);
    
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * ubo_data.size(), &ubo_data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //Setup the SSBO
    //Resizing and concatenation are needet to get memory layout as declared in the shaders
    //constexpr size_t max_drawable_id = 132;
    //quad_sizes.resize(max_drawable_id);
    //trim_flags.resize(max_drawable_id);
    //
    //std::vector<float> ssbo_data;
    //ssbo_data.insert(ssbo_data.end(), quad_sizes.begin(), quad_sizes.end());
    //ssbo_data.insert(ssbo_data.end(), trim_flags.begin(), trim_flags.end());

    //glGenBuffers(1, &m_SSBO);
    //
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
    //glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * ssbo_data.size(), &ssbo_data[0], GL_STATIC_DRAW);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  
}

void Clipmap::GenerateGrids(uint32_t level, float grid_size, float quad_size, int& drawable_id)
{
    //Center positions of particular grids within a level (single ring of the clipmap)
    std::vector<glm::vec2> centers;

    //Additional offsets
    std::vector<glm::vec2> offsets;

    const glm::vec2 top_left{ 0.0f, 1.0f };
    const glm::vec2 top_right{ 1.0f, 1.0f };
    const glm::vec2 bot_left{ 0.0f, 0.0f };
    const glm::vec2 bot_right{ 1.0f, 0.0f };

    //Edge flags
    std::vector<EdgeFlag> edge_flags;

    if (level == 0)
    {
        centers = { {-0.5f, 0.5f}, {0.5f, 0.5f},
                    {-0.5f,-0.5f}, {0.5f,-0.5f} };

        offsets = { top_left, top_right,
                   bot_left, bot_right };

        edge_flags = {LeftTop, RightTop,
                      LeftBot, RightBot};
    }

    else
    {
        centers = { {-1.5f, 1.5f}, {-0.5f, 1.5f}, { 0.5f, 1.5f}, { 1.5f, 1.5f},
                    {-1.5f, 0.5f},                               { 1.5f, 0.5f},
                    {-1.5f,-0.5f},                               { 1.5f,-0.5f},
                    {-1.5f,-1.5f}, {-0.5f,-1.5f}, { 0.5f,-1.5f}, { 1.5f,-1.5f} };

        offsets = { top_left, top_left, top_right, top_right,
                    top_left,                      top_right,
                    bot_left,                      bot_right,
                    bot_left, bot_left, bot_right, bot_right };

        edge_flags = { LeftTop, Top,    Top,    RightTop,
                       Left,                    Right,
                       Left,                    Right,
                       LeftBot, Bottom, Bottom, RightBot};
    }

    for (size_t i = 0; i < centers.size(); i++)
    {
        //For level zero the number of vertices needs to be twice as large (-1 is due to overlap)
        const uint32_t num_verts = (level == 0) ? 2 * m_VertsPerLine - 1 : m_VertsPerLine;

        const glm::vec2 center_pos = grid_size * centers[i] + quad_size * offsets[i];

        m_Grids.emplace_back();

        GenerateGrid(m_Grids.back(), num_verts, grid_size, center_pos, edge_flags[i]);

        m_Grids.back().GenGLBuffers();

        //Bounding box parameters
        const float bb_center_height = 0.45f;
        const float bb_vertical_extents = 0.55;

        const glm::vec3 center{ center_pos.x, bb_center_height, center_pos.y };
        const glm::vec3 extents{ 0.5f * grid_size, bb_vertical_extents, 0.5f * grid_size };

        m_Grids.back().BoundingBox.Center = center;
        m_Grids.back().BoundingBox.Extents = extents;

        //Current drawable id
        //m_Grids.back().DrawableID = drawable_id;
        //drawable_id++;
        m_Grids.back().DrawableID = (1+level);
    }
}

void Clipmap::GenerateFills(uint32_t level, float grid_size, float quad_size, int& drawable_id)
{
    //const float quad_size = std::pow(2, level) * m_BaseQuadSize;

    if (level == 0)
    {
        //Length of 4 grids with 2 overlaps
        const uint32_t num_verts = 4 * m_VertsPerLine - 2;

        const glm::vec2 origin_x{ -grid_size, 0.0f };
        const glm::vec2 origin_y{ 0.0f, -grid_size };

        m_Fills.emplace_back();
        GenerateStrip(m_Fills.back(), Orientation::Horizontal, num_verts, quad_size, origin_x, false, true);
        GenerateStrip(m_Fills.back(), Orientation::Vertical,   num_verts, quad_size, origin_y, false, true);
        m_Fills.back().GenGLBuffers();

        //m_Fills.back().DrawableID = drawable_id;
        //drawable_id++;
        m_Fills.back().DrawableID = (1+level);
    }

    else
    {
        const uint32_t num_verts = m_VertsPerLine;

        const glm::vec2 top{ 0.0f, grid_size + quad_size };
        const glm::vec2 bottom{ 0.0f, -2.0f * grid_size };
        const glm::vec2 left{ -2.0f * grid_size, 0.0f };
        const glm::vec2 right{ grid_size + quad_size, 0.0f };

        m_Fills.emplace_back();
        GenerateStrip(m_Fills.back(), Orientation::Vertical,   num_verts, quad_size, top,    false, false);
        GenerateStrip(m_Fills.back(), Orientation::Vertical,   num_verts, quad_size, bottom, false, false);
        GenerateStrip(m_Fills.back(), Orientation::Horizontal, num_verts, quad_size, left,   false, false);
        GenerateStrip(m_Fills.back(), Orientation::Horizontal, num_verts, quad_size, right,  false, false);
        m_Fills.back().GenGLBuffers();

        //m_Fills.back().DrawableID = drawable_id;
        //drawable_id++;
        m_Fills.back().DrawableID = (1+level);
    }
}

void Clipmap::GenerateTrims(uint32_t level, float grid_size, float quad_size, int& drawable_id)
{
    //Number of vertices in one line 
    //4 grids -3 because of overlap +1 because corner sticks out +1 because of fill meshes
    const uint32_t num_verts = 4 * m_VertsPerLine - 1;

    const float grid_corner = (level == 0) ? grid_size : 2.0f * grid_size;

    const glm::vec2 origin_x{ grid_corner, -grid_corner - quad_size };
    const glm::vec2 origin_y{ -grid_corner - quad_size, grid_corner };

    m_Trims.emplace_back();
    GenerateStrip(m_Trims.back(), Orientation::Vertical,   num_verts, quad_size, origin_x, true, false);
    GenerateStrip(m_Trims.back(), Orientation::Horizontal, num_verts, quad_size, origin_y, true, false);
    m_Trims.back().GenGLBuffers();

    //m_Trims.back().DrawableID = drawable_id;
    //drawable_id++;
    m_Trims.back().DrawableID = -(1+level);
}

uint32_t Clipmap::NumGridsPerLevel(uint32_t level)
{
    return (level == 0) ? 4 : 12;
}

uint32_t Clipmap::MaxGridIDUpTo(uint32_t level)
{
    return (level == 0) ? 0 : 4 + (level-1) * 12;
}

bool Clipmap::LevelShouldUpdate(uint32_t level, glm::vec2 curr, glm::vec2 prev) const
{
    const float scale = std::pow(2, level) * m_BaseQuadSize;

    glm::vec2 p_offset = prev - glm::mod(prev, scale);
    glm::vec2 c_offset = curr - glm::mod(curr, scale);

    return (p_offset.x != c_offset.x) || (p_offset.y != c_offset.y);
}

void Clipmap::RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding)
{
    for (const auto& grid : m_Grids)
    {
        grid.BindBufferBase(binding);
        shader->setUniform1i("uDrawableID", grid.DrawableID);
        shader->Dispatch(grid.VertexCount(), 1, 1);
    }

    for (const auto& fill : m_Fills)
    {
        fill.BindBufferBase(binding);
        shader->setUniform1i("uDrawableID", fill.DrawableID);
        shader->Dispatch(fill.VertexCount(), 1, 1);
    }

    for (const auto& trim : m_Trims)
    {
        trim.BindBufferBase(binding);
        shader->setUniform1i("uDrawableID", trim.DrawableID);
        shader->Dispatch(trim.VertexCount(), 1, 1);
    }

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Clipmap::RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev)
{
    uint32_t grid_id = 0;

    for (uint32_t level = 0; level < m_Levels; level++)
    {
        if (!LevelShouldUpdate(level, curr, prev))
            continue;

        for (uint32_t i = 0; i < NumGridsPerLevel(level); i++)
        {
            auto& grid = m_Grids[grid_id];

            grid.BindBufferBase(binding);
            shader->setUniform1i("uDrawableID", grid.DrawableID);
            shader->Dispatch(grid.VertexCount(), 1, 1);

            grid_id++;
        }

        auto& fill = m_Fills[level];
        
        fill.BindBufferBase(binding);
        shader->setUniform1i("uDrawableID", fill.DrawableID);
        shader->Dispatch(fill.VertexCount(), 1, 1);
        
        auto& trim = m_Trims[level];
        
        trim.BindBufferBase(binding);
        shader->setUniform1i("uDrawableID", trim.DrawableID);
        shader->Dispatch(trim.VertexCount(), 1, 1);
    }

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); 
}

//void Clipmap::BindSSBO(uint32_t binding)
//{
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_SSBO);
//}

void Clipmap::BindUBO(uint32_t binding)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_UBO);
}

void Clipmap::Draw(const std::shared_ptr<VertFragShader>& shader, const Camera& cam, float scale_y)
{
    for (const auto& grid : m_Grids)
    {
        if (cam.IsInFrustum(grid.BoundingBox, scale_y))
        {
            shader->setUniform1i("uDrawableID", grid.DrawableID);
            grid.Draw();
        }
    }

    for (const auto& fill : m_Fills)
    {
        shader->setUniform1i("uDrawableID", fill.DrawableID);
        fill.Draw();
    }

    for (const auto& trim : m_Trims)
    {
        shader->setUniform1i("uDrawableID", trim.DrawableID);
        trim.Draw();
    }
}