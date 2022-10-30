#pragma once

#include "Shader.h"
#include "GLUtils.h"

struct MaterialSettings{
    glm::vec3 Col1 = glm::vec3(0.5f);
    glm::vec3 Col2 = glm::vec3(0.8f);
};

class MaterialRenderer{
public:
    MaterialRenderer();
    ~MaterialRenderer();

    void Update();
    void OnImGui();
    void BindAlbedo(int id=0);
    void BindNormal(int id=0);

private:
    Quad m_Quad;
    FramebufferTexture m_Albedo, m_Normal;
    Shader m_AlbedoShader, m_NormalShader;

    MaterialSettings m_Settings;
};

bool operator==(const MaterialSettings& lhs, const MaterialSettings& rhs);
bool operator!=(const MaterialSettings& lhs, const MaterialSettings& rhs);
