//Clipmap Terrain, largely based around the following articles:
//https://mikejsavage.co.uk/blog/geometry-clipmaps.html
//https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry

#pragma once

#include "Shader.h"
#include "Camera.h"
#include "ResourceManager.h"

class Drawable{
public:
    Drawable();
    ~Drawable();

    void GenBuffers();
    void DispatchCompute();
    void Draw();

    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int ElementCount = 0;
    std::vector<float> VertexData;
    std::vector<unsigned int> IndexData;
};

class ClipmapRing {
public:
    ClipmapRing(int N, float L, unsigned int level);
    ~ClipmapRing();

    void DispatchCompute();
    void Draw(const Camera& cam, float scale_y);
private:
    std::vector<Drawable> m_Grid;
    std::vector<AABB> m_Bounds;

    Drawable m_FillX, m_FillY;
};

class Clipmap {
public:
    Clipmap(ResourceManager& manager);

    void Init(int subdivisions, int levels);

    void DisplaceVertices(float scale_xz, float scale_y,
                          glm::vec2 pos);

    void DisplaceVertices(float scale_xz, float scale_y,
                          glm::vec2 curr, glm::vec2 prev);

    void BindAndDraw(const Camera& cam, float scale_y);

private:
    float m_L = 4.0f, m_BaseOffset = 1.0f;

    std::vector<ClipmapRing> m_LodLevels;
    std::shared_ptr<ComputeShader> m_DisplaceShader;

    ResourceManager& m_ResourceManager;
};
