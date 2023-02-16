//Clipmap Terrain, largely based around the following articles:
//https://mikejsavage.co.uk/blog/geometry-clipmaps.html
//https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry
//https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

#pragma once

#include "Shader.h"
#include "Camera.h"

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

struct Plane {
    glm::vec3 Origin = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Normal = { 0.0f, 1.0f, 0.0f };
};

class Frustum {
public:
    Frustum(const Camera& camera, float aspect);
    ~Frustum() = default;

    Plane Top, Bottom, Left, Right, Near, Far;
};

class AABB {
public:
    AABB(float x, float y, float l);
    ~AABB() = default;

    bool IsInFrustum(const Frustum& frustum, float scale_y);
private:
    glm::vec3 m_Center, m_Extents;

    bool IsInFront(const Plane& plane, float scale_y);
};

class ClipmapRing {
public:
    ClipmapRing(int N, float L, unsigned int level);
    ~ClipmapRing();

    void DispatchCompute();
    void Draw(const Frustum& frustum, float scale_y);
private:
    std::vector<Drawable> m_Grid;
    std::vector<AABB> m_Bounds;

    Drawable m_FillX, m_FillY;
};

class ClipmapRenderer {
public:
    ClipmapRenderer();
    ~ClipmapRenderer();

    void Init(int subdivisions, int levels);
    void DisplaceVertices(float scale_xz, float scale_y,
                          float offset_x, float offset_z);
    void BindAndDraw(const Camera& cam, float aspect, float scale_y);

    void PrintFrustum(const Camera& cam, float aspect);
private:
    float m_L = 4.0f;

    std::vector<ClipmapRing> m_LodLevels;
    Shader m_DisplaceShader;
};
