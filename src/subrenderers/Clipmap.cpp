#include "Clipmap.h"
#include "Profiler.h"

#include "glad/glad.h"

#include <algorithm>
#include <iostream>

#include "ImGuiUtils.h"

void Drawable::Draw() const
{
    glDrawElementsBaseVertex(
        GL_TRIANGLES,
        IndexCount,
        GL_UNSIGNED_INT,
 	    (void*)(sizeof(uint32_t) * FirstIndex),
        BaseVertex
    );
}

void Drawable::BindBufferRange(uint32_t vertex_buffer, uint32_t binding) const
{
    //Position (in bytes) of this drawable's data in the global vertex buffer
    const auto start = sizeof(ClipmapVertex) * BaseVertex;
    const auto size  = sizeof(ClipmapVertex) * VertexCount;

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding, vertex_buffer, start, size);
}

//We will only store information about a vertex being on
//vertical/horizontal edge, or not being at an edge at all.
//This doesn't describe corners well, but that's not a problem,
//since by construction of the clipmap, the corners are always aligned
//with the higher levels by default.

enum EdgeData {
    NoEdge = 0, Horizontal = 1, Vertical = 2
};

static uint32_t PackAuxData(bool is_trim, EdgeData edge_data, uint32_t lvl)
{
    const uint32_t trim = static_cast<uint32_t>(is_trim);
    const uint32_t edge = static_cast<uint32_t>(edge_data) << 1;
    const uint32_t level = lvl << 3;

    return trim | edge | level;
}

//Denotes position of the current grid in its ring of the clipmap
enum GridPosFlag {
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

struct GridInfo{
    uint32_t VertsPerLine;
    float SideLength;
    glm::vec2 GlobalOffset;
    GridPosFlag PosFlag;
    uint32_t Level;
};

static void GenerateGrid(std::vector<ClipmapVertex>& verts,
        std::vector<uint32_t>& elements,
        Drawable& drawable,
        GridInfo info)
{
    const auto vert_count = info.VertsPerLine * info.VertsPerLine;
    //1 quad = 6 indices:
    const auto idx_count = 6 * (info.VertsPerLine - 1) * (info.VertsPerLine - 1);

    const float quad_size = info.SideLength / static_cast<float>(info.VertsPerLine - 1);

    //Setup draw command data
    drawable.IndexCount = idx_count;
    drawable.InstanceCount = 1;
    drawable.BaseVertex = static_cast<int>(verts.size());
    drawable.FirstIndex = static_cast<uint32_t>(elements.size());
    drawable.BaseInstance = 0;
    drawable.VertexCount = vert_count;

    auto GetOffset = [info, quad_size](uint32_t i)
    {
        return glm::vec2{
            quad_size * static_cast<float>(i % info.VertsPerLine),
            quad_size * static_cast<float>(i / info.VertsPerLine)
        };
    };

    auto GetEdgeData = [info](uint32_t i) -> EdgeData
    {
        const auto idx = i % info.VertsPerLine;
        const auto idy = i / info.VertsPerLine;

        const bool left_edge   = (idx == 0);
        const bool right_edge  = (idx == info.VertsPerLine - 1);
        const bool bottom_edge = (idy == 0);
        const bool top_edge    = (idy == info.VertsPerLine - 1);

        if (left_edge && ((info.PosFlag & Left) != None))
            return Vertical;

        if (right_edge && ((info.PosFlag & Right) != None))
            return Vertical;

        if (top_edge && ((info.PosFlag & Top) != None))
            return Horizontal;

        if (bottom_edge && ((info.PosFlag & Bottom) != None))
            return Horizontal;

        return NoEdge;
    };

    //Vertex data:
    const glm::vec2 origin = glm::vec2(-info.SideLength / 2.0f)
                           + info.GlobalOffset;

    for (uint32_t i=0; i<vert_count; i++)
    {
        const glm::vec2 offset = GetOffset(i);

        verts.push_back(ClipmapVertex{
            origin.x + offset.x, 0.0f, origin.y + offset.y, //position
            PackAuxData(false, GetEdgeData(i), info.Level)
        });

    }

    //Index data:
    for (uint32_t i=0; i<vert_count; i++)
    {
        const auto n = info.VertsPerLine;

        uint32_t ix = i % n;
        uint32_t iy = i / n;

        if (ix == n - 1) continue;
        if (iy == n - 1) continue;

        elements.push_back(i);
        elements.push_back(i+1);
        elements.push_back(i+n);

        elements.push_back(i+1);
        elements.push_back(i+1+n);
        elements.push_back(i+n);
    }
}

enum class StripOrientation {
    Horizontal, Vertical
};

struct StripInfo{
    uint32_t VertsPerLine;
    float QuadSize;
    StripOrientation Orientation;
    glm::vec2 GlobalOffset;
    bool IsTrim;
    uint32_t Level;
};

static void GenerateStrip(std::vector<ClipmapVertex>& verts,
        std::vector<uint32_t>& elements,
        Drawable& drawable,
        StripInfo info)
{
    const auto start_id = drawable.VertexCount;

    const auto vert_count = 2 * info.VertsPerLine;
    const auto idx_count = 6 * (info.VertsPerLine - 1);

    drawable.VertexCount += vert_count;

    //Setup draw command data
    bool first_call = drawable.IndexCount == 0;

    drawable.IndexCount += idx_count;

    if (first_call)
    {
        drawable.InstanceCount = 1;
        drawable.BaseVertex = static_cast<int>(verts.size());
        drawable.FirstIndex = static_cast<uint32_t>(elements.size());
        drawable.BaseInstance = 0;
    }

    auto GenOffset = [info](uint32_t i)
    {
        const glm::vec2 res{
            info.QuadSize * static_cast<float>(i/2),
            info.QuadSize * static_cast<float>(i%2)
        };

        if (info.Orientation == StripOrientation::Vertical)
            return glm::vec2{ res.y, res.x };
        else
            return res;
    };

    auto GetEdgeData = [info](uint32_t i) -> EdgeData
    {
        bool first_two = (i == 0 || i == 1);
        bool last_two = (i == 2 * info.VertsPerLine - 1 || i == 2 * info.VertsPerLine - 2);

        bool horizontal = (info.Orientation == StripOrientation::Horizontal);

        auto orientation_aligned  = horizontal ? Horizontal : Vertical;
        auto orientation_opposite = horizontal ? Vertical : Horizontal;

        if (info.IsTrim)
        {
            if (first_two || last_two)
                return orientation_opposite;

            else
                return orientation_aligned;
        }

        else
        {
            //On level zero edge corresponds to both start and end.
            if (info.Level == 0)
            {
                if (first_two || last_two)
                    return orientation_opposite;
            }

            //On levels higher than zero edge will correspond to either the first two
            //or the last two vertices.
            else
            {
                bool edge_at_start = (horizontal  && (info.GlobalOffset.x < 0.0f))
                                || ((!horizontal) && (info.GlobalOffset.y < 0.0f));

                if (edge_at_start && first_two)
                    return orientation_opposite;

                else if (!edge_at_start && last_two)
                    return orientation_opposite;
            }
        }

        return NoEdge;
    };

    //Vertex data:
    for (uint32_t i=0; i<vert_count; i++)
    {
        const glm::vec2 origin = info.GlobalOffset;
        const glm::vec2 offset = GenOffset(i);

        verts.push_back(ClipmapVertex{
            origin.x + offset.x, 0.0f, origin.y + offset.y, //position
            PackAuxData(info.IsTrim, GetEdgeData(i), info.Level)
        });
    }

    //Index data:
    for (uint32_t i=0; i<info.VertsPerLine-1; i++)
    {
        switch (info.Orientation)
        {
            case StripOrientation::Horizontal:
            {
                elements.push_back(start_id + 2 * i);
                elements.push_back(start_id + 2 * i + 2);
                elements.push_back(start_id + 2 * i + 1);

                elements.push_back(start_id + 2 * i + 1);
                elements.push_back(start_id + 2 * i + 2);
                elements.push_back(start_id + 2 * i + 3);

                break;
            }
            case StripOrientation::Vertical:
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
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

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
        const float scale = (lvl == 0) ? 2.0f : static_cast<float>(std::pow(2.0f, lvl));

        return scale * m_BaseGridSize;
    };

    auto getQuadSize = [this](uint32_t lvl) -> float
    {
        return static_cast<float>(std::pow(2, lvl)) * m_BaseQuadSize;
    };

    //Currently the ordering here is not critical, but we maintain
    //it structured in this way for greater future flexibility
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateGrids(lvl, getGridSize(lvl), getQuadSize(lvl));
    }
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateFills(lvl, getGridSize(lvl), getQuadSize(lvl));
    }
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        GenerateTrims(lvl, getGridSize(lvl), getQuadSize(lvl));
    }

    //Setup UBO data. Store only quad sizes for each level:
    for (uint32_t lvl = 0; lvl < levels; lvl++)
    {
        m_UBOData.push_back(getQuadSize(lvl));
        //Padding needed since ubo's can only use std140 layout
        //Where arrays have 16 byte alignment
        m_UBOData.push_back(0.0f);
        m_UBOData.push_back(0.0f);
        m_UBOData.push_back(0.0f);
    }

    GenGLBuffers();

    //Free buffer memory on cpu side
    m_VertexData.clear();
    m_IndexData.clear();
    m_UBOData.clear();

    std::vector<ClipmapVertex>().swap(m_VertexData);
    std::vector<uint32_t>().swap(m_IndexData);
    std::vector<float>().swap(m_UBOData);
}

void Clipmap::GenerateGrids(uint32_t level, float grid_size, float quad_size)
{
    //Center positions of particular grids within a level (single ring of the clipmap)
    std::vector<glm::vec2> centers;
    //Additional offsets
    std::vector<glm::vec2> offsets;
    //Edge flags
    std::vector<GridPosFlag> pos_flags;

    const glm::vec2 top_left{ 0.0f, 1.0f };
    const glm::vec2 top_right{ 1.0f, 1.0f };
    const glm::vec2 bot_left{ 0.0f, 0.0f };
    const glm::vec2 bot_right{ 1.0f, 0.0f };

    if (level == 0)
    {
        centers = { {-0.5f, 0.5f}, {0.5f, 0.5f},
                    {-0.5f,-0.5f}, {0.5f,-0.5f} };

        offsets = { top_left, top_right,
                   bot_left, bot_right };

        pos_flags = {LeftTop, RightTop,
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

        pos_flags = { LeftTop, Top,    Top,    RightTop,
                      Left,                    Right,
                      Left,                    Right,
                      LeftBot, Bottom, Bottom, RightBot};
    }

    for (size_t i = 0; i < centers.size(); i++)
    {
        //For level zero the number of vertices needs to be twice as large (-1 is due to overlap)
        const uint32_t num_verts = (level == 0) ? 2 * m_VertsPerLine - 1 : m_VertsPerLine;

        const glm::vec2 center_pos = grid_size * centers[i] + quad_size * offsets[i];

        const GridInfo grid_info{num_verts, grid_size, center_pos, pos_flags[i], level};

        m_Grids.emplace_back();
        GenerateGrid(m_VertexData, m_IndexData, m_Grids.back(), grid_info);

        //Bounding box parameters
        const float bb_center_height = 0.45f;
        const float bb_vertical_extents = 0.55f;

        const glm::vec3 center{ center_pos.x, bb_center_height, center_pos.y };
        const glm::vec3 extents{ 0.5f * grid_size, bb_vertical_extents, 0.5f * grid_size };

        m_Grids.back().BoundingBox.Center = center;
        m_Grids.back().BoundingBox.Extents = extents;
    }
}

void Clipmap::GenerateFills(uint32_t level, float grid_size, float quad_size)
{
    if (level == 0)
    {
        //Length of 4 grids with 2 overlaps
        const uint32_t num_verts = 4 * m_VertsPerLine - 2;

        const glm::vec2 origin_x{ -grid_size, 0.0f };
        const glm::vec2 origin_y{ 0.0f, -grid_size };

        const StripInfo info1{num_verts, quad_size, StripOrientation::Horizontal, origin_x, false, level};
        const StripInfo info2{num_verts, quad_size, StripOrientation::Vertical,   origin_y, false, level};

        m_Fills.emplace_back();
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info1);
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info2);
    }

    else
    {
        const uint32_t num_verts = m_VertsPerLine;

        const glm::vec2 top{ 0.0f, grid_size + quad_size };
        const glm::vec2 bottom{ 0.0f, -2.0f * grid_size };
        const glm::vec2 left{ -2.0f * grid_size, 0.0f };
        const glm::vec2 right{ grid_size + quad_size, 0.0f };

        const StripInfo info1{num_verts, quad_size, StripOrientation::Vertical, top, false, level};
        const StripInfo info2{num_verts, quad_size, StripOrientation::Vertical, bottom, false, level};
        const StripInfo info3{num_verts, quad_size, StripOrientation::Horizontal, left, false, level};
        const StripInfo info4{num_verts, quad_size, StripOrientation::Horizontal, right, false, level};

        m_Fills.emplace_back();
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info1);
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info2);
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info3);
        GenerateStrip(m_VertexData, m_IndexData, m_Fills.back(), info4);
    }
}

void Clipmap::GenerateTrims(uint32_t level, float grid_size, float quad_size)
{
    //Number of vertices in one line
    //4 grids -3 because of overlap +1 because corner sticks out +1 because of fill meshes
    const uint32_t num_verts = 4 * m_VertsPerLine - 1;

    const float grid_corner = (level == 0) ? grid_size : 2.0f * grid_size;

    const glm::vec2 origin_x{ grid_corner, -grid_corner - quad_size };
    const glm::vec2 origin_y{ -grid_corner - quad_size, grid_corner };

    const StripInfo info1{num_verts, quad_size, StripOrientation::Vertical, origin_x, true, level};
    const StripInfo info2{num_verts, quad_size, StripOrientation::Horizontal, origin_y, true, level};

    m_Trims.emplace_back();
    GenerateStrip(m_VertexData, m_IndexData, m_Trims.back(), info1);
    GenerateStrip(m_VertexData, m_IndexData, m_Trims.back(), info2);
}

void Clipmap::GenGLBuffers()
{
    //Generate Vertex Array and Buffer + Element Buffer
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ClipmapVertex) * m_VertexData.size(),
                 &m_VertexData[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_IndexData.size(),
                 &m_IndexData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ClipmapVertex), (void*)(offsetof(ClipmapVertex, PosX)));
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(ClipmapVertex), (void*)(offsetof(ClipmapVertex, AuxData)));
    glEnableVertexAttribArray(1);

    //Generate the UBO
    glGenBuffers(1, &m_UBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * m_UBOData.size(), &m_UBOData[0], GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
    const float scale = static_cast<float>(std::pow(2, level)) * m_BaseQuadSize;

    glm::vec2 p_offset = prev - glm::mod(prev, scale);
    glm::vec2 c_offset = curr - glm::mod(curr, scale);

    return (p_offset.x != c_offset.x) || (p_offset.y != c_offset.y);
}

void Clipmap::RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding)
{
    glBindVertexArray(m_VAO);

    for (const auto& grid : m_Grids)
    {
        grid.BindBufferRange(m_VBO, binding);
        shader->Dispatch(grid.VertexCount, 1, 1);
    }

    for (const auto& fill : m_Fills)
    {
        fill.BindBufferRange(m_VBO, binding);
        shader->Dispatch(fill.VertexCount, 1, 1);
    }

    for (const auto& trim : m_Trims)
    {
        trim.BindBufferRange(m_VBO, binding);
        shader->Dispatch(trim.VertexCount, 1, 1);
    }

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Clipmap::RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev)
{
    glBindVertexArray(m_VAO);

    for (uint32_t level = 0; level < m_Levels; level++)
    {
        if (!LevelShouldUpdate(level, curr, prev))
            continue;

        for (uint32_t i = 0; i < NumGridsPerLevel(level); i++)
        {
            auto& grid = m_Grids[MaxGridIDUpTo(level) + i];

            grid.BindBufferRange(m_VBO, binding);
            shader->Dispatch(grid.VertexCount, 1, 1);
        }

        auto& fill = m_Fills[level];

        fill.BindBufferRange(m_VBO, binding);
        shader->Dispatch(fill.VertexCount, 1, 1);

        auto& trim = m_Trims[level];

        trim.BindBufferRange(m_VBO, binding);
        shader->Dispatch(trim.VertexCount, 1, 1);
    }

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Clipmap::BindUBO(uint32_t binding)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_UBO);
}

void Clipmap::BindBuffers(uint32_t ubo_binding)
{
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, ubo_binding, m_UBO);
}

void Clipmap::Draw(const Camera& cam, float scale_y)
{
    for (const auto& grid : m_Grids)
    {
        if (cam.IsInFrustum(grid.BoundingBox, scale_y))
            grid.Draw();
    }

    for (const auto& fill : m_Fills)
        fill.Draw();

    for (const auto& trim : m_Trims)
        trim.Draw();
}

void Clipmap::ImGuiDebugCulling(const Camera& cam, float scale_y, bool& open)
{
    ImGui::Begin("Frustum culling debug", &open);

    static int max_lvl = m_Levels;
    ImGui::Columns(2, "###col");
    ImGuiUtils::ColSliderInt("Max level", &max_lvl, 1, m_Levels);
	ImGui::Columns(1, "###col");

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    auto ImToGlm = [](ImVec2 v) {return glm::vec2(v.x, v.y);};

	const glm::vec2 draw_start = ImToGlm(ImGui::GetCursorScreenPos());
	const glm::vec2 draw_size  = ImToGlm(ImGui::GetContentRegionAvail());

    const float clipmap_size = m_BaseGridSize * static_cast<float>(std::pow(2, max_lvl + 1));

    auto WorldToWindow = [&](glm::vec3 pos){
        const glm::vec2 pos2{pos.x, pos.z};
        const glm::vec2 normalized = pos2/clipmap_size + 0.5f;

        return draw_start + std::min(draw_size.x, draw_size.y) * normalized;
    };

    ImGui::BeginChild("CPU Timing", ImVec2(draw_size.x, draw_size.y), true);

    for (int lvl=0; lvl<max_lvl; lvl++)
    {
        for (uint32_t i=0; i<NumGridsPerLevel(lvl); i++)
        {
            const auto& grid = m_Grids[MaxGridIDUpTo(lvl) + i];
            const auto& aabb = grid.BoundingBox;

            const bool in_frustum = cam.IsInFrustum(aabb, scale_y);

            const ImU32 fill_color = in_frustum ? IM_COL32(100, 250, 100, 255) : IM_COL32(64, 64, 64, 255);
            const ImU32 border_color = IM_COL32(0,0,0,255);

            const glm::vec2 min_pos = WorldToWindow(aabb.Center - aabb.Extents);
            const glm::vec2 max_pos = WorldToWindow(aabb.Center + aabb.Extents);

            drawList->AddRectFilled(ImVec2(min_pos.x, min_pos.y), ImVec2(max_pos.x, max_pos.y), fill_color);
            drawList->AddRect(ImVec2(min_pos.x, min_pos.y), ImVec2(max_pos.x, max_pos.y), border_color);
        }
    }

    ImGui::EndChild();

    ImGui::End();
}