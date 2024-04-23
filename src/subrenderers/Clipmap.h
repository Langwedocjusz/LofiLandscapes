#pragma once

//Clipmap Terrain, largely based around the following articles:
//https://mikejsavage.co.uk/blog/geometry-clipmaps.html
//https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry

#include "Shader.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "MapGenerator.h"

#include <cstdint>

struct ClipmapVertex {
    float PosX, PosY, PosZ;
    //Aux Data contains:
    //trim flag - 1 bit
    //edge flag - 2 bits
    //lod level - the rest
    uint32_t AuxData;
};

class Drawable{
public:
    uint32_t  IndexCount = 0;
    uint32_t  InstanceCount = 0;
    uint32_t  FirstIndex = 0;
    int       BaseVertex = 0;
    uint32_t  BaseInstance = 0;
    uint32_t  VertexCount = 0;

    //Assumes all apropriate buffers are already bound
    void Draw() const;

    void BindBufferRange(uint32_t vertex_buffer, uint32_t binding) const;
};

class DrawableWithBounding : public Drawable{
public:
    AABB BoundingBox;
};

class Clipmap {
public:
    ~Clipmap();

    void Init(uint32_t subdivisions, uint32_t levels);

    const std::vector<DrawableWithBounding>& getGrids() const { return m_Grids; }
    const std::vector<Drawable>& getTrims() const { return m_Trims; }
    const std::vector<Drawable>& getFills() const { return m_Fills; }

    //Binds Uniform Buffer Object with data needed for drawing
    void BindUBO(uint32_t binding);

    //Binds both vertex/element buffers and the ubo
    void BindBuffers(uint32_t ubo_binding);

    //Dispatches the compute shader for all grids/fills of the clipmap
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument for vertex buffer
    void RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding);

    //Conditionally runs the compute shader, for those grids/fill that should be updated
    //afted a change in the camera position.
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument for vertex buffer
    void RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev);

    //Calls draw function on all grids/fills, using frustum culling
    void Draw(const std::shared_ptr<VertFragShader>& shader, const Camera& cam, float scale_y);

    void ImGuiDebugCulling(const Camera& cam, float scale_y, bool& open);

    bool LevelShouldUpdate(uint32_t level, glm::vec2 curr, glm::vec2 prev) const;

    static uint32_t NumGridsPerLevel(uint32_t level);
    static uint32_t MaxGridIDUpTo(uint32_t level);

private:
    void GenerateGrids(uint32_t level, float grid_size, float quad_size);
    void GenerateFills(uint32_t level, float grid_size, float quad_size);
    void GenerateTrims(uint32_t level, float grid_size, float quad_size);

    void GenGLBuffers();

    uint32_t m_Levels;
    uint32_t m_VertsPerLine;
    float m_BaseGridSize = 4.0f;
    float m_BaseQuadSize;

    std::vector<DrawableWithBounding> m_Grids;
    std::vector<Drawable> m_Trims, m_Fills;

    //GL handles:
    uint32_t m_VAO = 0, m_VBO = 0, m_EBO = 0;
    uint32_t m_UBO = 0;

    //Temp GL Buffer data:
    std::vector<ClipmapVertex> m_VertexData;
    std::vector<uint32_t> m_IndexData;
    std::vector<float> m_UBOData;
};
