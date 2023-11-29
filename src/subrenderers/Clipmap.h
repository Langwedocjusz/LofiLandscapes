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
    float Padding1;
    float QuadSize;
    float EdgeFlag;
    float TrimFlag;
    float Padding2;
};

class Drawable{
public:
    ~Drawable();

    void GenGLBuffers();
    void BindBufferBase(uint32_t id = 0) const;
    void Draw() const;
    
    std::vector<ClipmapVertex> VertexData;
    std::vector<uint32_t> IndexData;

    size_t VertexCount() const { return VertexData.size(); }
    size_t ElementCount() const { return IndexData.size(); }
protected:
    uint32_t m_VAO = 0, m_VBO = 0, m_EBO = 0;
};

class DrawableWithBounding : public Drawable {
public:
    AABB BoundingBox;
};

class Clipmap {
public:
    void Init(uint32_t subdivisions, uint32_t levels);

    const std::vector<DrawableWithBounding>& getGrids() const { return m_Grids; }
    const std::vector<Drawable>& getTrims() const { return m_Trims; }
    const std::vector<Drawable>& getFills() const { return m_Fills; }

    //Dispatches the compute shader for all grids/fills of the clipmap
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument
    void RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding);

    //Conditionally runs the compute shader, for those grids/fill that should be updated
    //afted a change in the camera position.
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument
    void RunCompute(const std::shared_ptr<ComputeShader>& shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev);

    //Calls draw function on all grids/fills, using frustum culling
    void Draw(const Camera& cam, float scale_y);

    bool LevelShouldUpdate(uint32_t level, glm::vec2 curr, glm::vec2 prev) const;

    static uint32_t NumGridsPerLevel(uint32_t level);
    static uint32_t MaxGridIDUpTo(uint32_t level);

private:
    void GenerateGrids(uint32_t level, float grid_size, float quad_size);
    void GenerateFills(uint32_t level, float grid_size, float quad_size);
    void GenerateTrims(uint32_t level, float grid_size, float quad_size);

    float m_BaseGridSize = 4.0f;

    float m_BaseQuadSize;

    uint32_t m_Levels;
    uint32_t m_VertsPerLine;

    std::vector<DrawableWithBounding> m_Grids;
    std::vector<Drawable> m_Trims, m_Fills;
};
