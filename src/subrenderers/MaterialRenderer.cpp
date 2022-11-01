#include "MaterialRenderer.h"

#include "glad/glad.h"
#include "imgui.h"

MaterialRenderer::MaterialRenderer()
    : m_AlbedoShader("res/shaders/albedo.glsl")
    , m_NormalShader("res/shaders/mnormal.glsl")
{
    TextureSpec spec = TextureSpec{
        512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Albedo.Initialize(spec);
    m_Albedo.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_Normal.Initialize(spec);
    m_Normal.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);
}

MaterialRenderer::~MaterialRenderer() {

}

void MaterialRenderer::Update() { 
    const int res = m_Normal.getSpec().Resolution;
    
    //Draw to normal:
    m_Normal.BindImage(0, 0);

    m_NormalShader.Bind();
    m_NormalShader.setUniform1i("uResolution", res);

    glDispatchCompute(res/32, res/32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    m_Normal.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    //Draw to albedo:
    m_Albedo.BindImage(0, 0);
    
    m_AlbedoShader.Bind();
    m_AlbedoShader.setUniform1i("uResolution", res);
    m_AlbedoShader.setUniform3f("uCol1", m_Settings.Col1);
    m_AlbedoShader.setUniform3f("uCol2", m_Settings.Col2);

    glDispatchCompute(res/32, res/32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    m_Albedo.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);
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
    m_Albedo.Bind(id);
}

void MaterialRenderer::BindNormal(int id) {
    m_Normal.Bind(id);
}

bool operator==(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return (lhs.Col1 == rhs.Col1) && (lhs.Col2 == rhs.Col2);
}

bool operator!=(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return !(lhs==rhs);
}
