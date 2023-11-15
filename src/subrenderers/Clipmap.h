//Clipmap Terrain, largely based around the following articles:
//https://mikejsavage.co.uk/blog/geometry-clipmaps.html
//https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry

#pragma once

#include "Shader.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "MapGenerator.h"

#include <cstdint>

class Drawable{
public:
    ~Drawable();

    void GenGLBuffers();
    void BindBufferBase(uint32_t id = 0) const;
    void Draw() const;
    
    std::vector<float> VertexData;
    std::vector<uint32_t> IndexData;
    uint32_t VertexCount = 0, ElementCount = 0;
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
    const std::vector<Drawable>& getFills() const { return m_Fills; }

    //Dispatches the compute shader for all grids/fills of the clipmap
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument
    void RunCompute(std::shared_ptr<ComputeShader> shader, uint32_t binding);

    //Conditionally runs the compute shader, for those grids/fill that should be updated
    //afted a change in the camera position.
    //Each time the shader is dispatched with (VertexCount, 1, 1) invocations 
    //Binding is forwarded as glBindBufferBase argument
    void RunCompute(std::shared_ptr<ComputeShader> shader, uint32_t binding, glm::vec2 curr, glm::vec2 prev);

    static uint32_t NumGridsPerLevel(uint32_t level);
    static uint32_t NumFillsPerLevel(uint32_t level);

    static uint32_t MaxGridIDUpTo(uint32_t level);
    static uint32_t MaxFillIDUpTo(uint32_t level);
    
    bool LevelShouldUpdate(uint32_t level, glm::vec2 curr, glm::vec2 prev) const;

private:
    float m_BaseSideLength = 4.0f;
    float m_VertsPerLine, m_BaseOffset;

    uint32_t m_Levels = 0;

    std::vector<DrawableWithBounding> m_Grids;
    std::vector<Drawable> m_Fills;
};
