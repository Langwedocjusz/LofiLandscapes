#include "MaterialRenderer.h"

#include "glad/glad.h"
#include "imgui.h"

MaterialRenderer::MaterialRenderer()
    : m_AlbedoShader("res/shaders/quad.vert", "res/shaders/albedo.frag")
    , m_NormalShader("res/shaders/quad.vert", "res/shaders/mnormal.frag")
{
    TextureSpec spec = TextureSpec{
        512, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Albedo.Initialize(spec);
    m_Normal.Initialize(spec);
}

MaterialRenderer::~MaterialRenderer() {

}

void MaterialRenderer::Update() {
    glViewport(0, 0, 512, 512);
    
    //Draw to normal:
    m_Normal.BindFBO();

    glClearColor(0.5f, 1.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_NormalShader.Bind();
    m_Quad.Draw();

    //Draw to albedo:
    m_Normal.BindTex();
    m_Albedo.BindFBO();

    glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_AlbedoShader.Bind();
    m_AlbedoShader.setUniform3f("uCol1", m_Settings.Col1);
    m_AlbedoShader.setUniform3f("uCol2", m_Settings.Col2);

    m_Quad.Draw();
}

void MaterialRenderer::OnImGui() {
    MaterialSettings temp = m_Settings;

    ImGui::Begin("Material");
    ImGui::ColorEdit3("Color1", glm::value_ptr(temp.Col1));
    ImGui::ColorEdit3("Color2", glm::value_ptr(temp.Col2));
    ImGui::End();

    if (temp != m_Settings) {
        m_Settings = temp;
        Update();
    }
}

void MaterialRenderer::BindAlbedo(int id) {
    m_Albedo.BindTex(id);
}

void MaterialRenderer::BindNormal(int id) {
    m_Normal.BindTex(id);
}

bool operator==(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return (lhs.Col1 == rhs.Col1) && (lhs.Col2 == rhs.Col2);
}

bool operator!=(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return !(lhs==rhs);
}
