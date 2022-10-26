#include "MapRenderer.h"

#include "glad/glad.h"

#include "imgui.h"

#include <iostream>

MapRenderer::MapRenderer()
    : m_HeightmapShader("res/shaders/quad.vert", "res/shaders/height.frag"),
      m_NormalmapShader("res/shaders/quad.vert", "res/shaders/normal.frag"),
      m_ShadowmapShader("res/shaders/quad.vert", "res/shaders/shadow.frag")
{
    //Quad (GL Buffers only):
    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glGenBuffers(1, &m_QuadEBO);

    glBindVertexArray(m_QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_QuadVertexData), 
            &m_QuadVertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_QuadIndexData), 
                    &m_QuadIndexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //-----Heightmap
    //Framebuffer
    const int tex_res = m_HeightSettings.Resolution;
    
    glGenFramebuffers(1, &m_HeightmapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HeightmapFBO);

    //Texture
    glGenTextures(1, &m_HeightmapTexture);
    glBindTexture(GL_TEXTURE_2D, m_HeightmapTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_res, tex_res, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float height_border[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, height_border);

    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_HeightmapTexture, 0);

    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    std::cerr << "FRAMEBUFFER NOT READY \n";

    //-----Normal map:
    //Framebuffer
    glGenFramebuffers(1, &m_NormalmapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_NormalmapFBO);

    //Texture
    glGenTextures(1, &m_NormalmapTexture);
    glBindTexture(GL_TEXTURE_2D, m_NormalmapTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_res, tex_res, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //Pointing up (0,1,0), after compression -> (0.5, 1.0, 0.5);
    float normal_border[4] = {0.5f, 1.0f, 0.5f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, normal_border);
    
    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_NormalmapTexture, 0);

    //-----Shadow map
    //Framebuffer
    glGenFramebuffers(1, &m_ShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);

    //Texture
    const int shadow_res = m_ShadowSettings.Resolution;

    glGenTextures(1, &m_ShadowTexture);
    glBindTexture(GL_TEXTURE_2D, m_ShadowTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, shadow_res, shadow_res, 0, 
                 GL_RED, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float shadow_border[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, shadow_border);
    
    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_ShadowTexture, 0);

    //Update
    m_UpdateFlags = m_UpdateFlags | UpdateFlags::Height;
    m_UpdateFlags = m_UpdateFlags | UpdateFlags::Normal;
    m_UpdateFlags = m_UpdateFlags | UpdateFlags::Shadow;
}

MapRenderer::~MapRenderer() {
    glDeleteFramebuffers(1, &m_HeightmapFBO);
    glDeleteFramebuffers(1, &m_NormalmapFBO);
    glDeleteFramebuffers(1, &m_ShadowFBO);
}

void MapRenderer::BindGeometry() {
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
}

void MapRenderer::Draw() {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void MapRenderer::UpdateHeight() {
    glViewport(0, 0, m_HeightSettings.Resolution, 
                     m_HeightSettings.Resolution);

    glBindFramebuffer(GL_FRAMEBUFFER, m_HeightmapFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_HeightmapShader.Bind();
    m_HeightmapShader.setUniform1i("uOctaves", m_HeightSettings.Octaves);
    m_HeightmapShader.setUniform2f("uOffset" , m_HeightSettings.Offset[0],
                                               m_HeightSettings.Offset[1]);
    BindGeometry();
    Draw();

    BindHeightmap();
    glGenerateMipmap(GL_TEXTURE_2D);
}

void MapRenderer::UpdateNormal() {
    glViewport(0, 0, m_HeightSettings.Resolution, 
                     m_HeightSettings.Resolution);
    
    BindHeightmap();
    glBindFramebuffer(GL_FRAMEBUFFER, m_NormalmapFBO);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_NormalmapShader.Bind();
    m_NormalmapShader.setUniform1f("uScaleXZ", m_ScaleSettings.ScaleXZ);
    m_NormalmapShader.setUniform1f("uScaleY" , m_ScaleSettings.ScaleY );
    
    BindGeometry();
    Draw();
}

void MapRenderer::UpdateShadow(float theta, float phi) {
    glViewport(0, 0, m_ShadowSettings.Resolution, 
                     m_ShadowSettings.Resolution);
    
    BindHeightmap();
    glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    m_ShadowmapShader.Bind();
    m_ShadowmapShader.setUniform1f("uTheta", theta);
    m_ShadowmapShader.setUniform1f("uPhi", phi);
    m_ShadowmapShader.setUniform1f("uScaleXZ", m_ScaleSettings.ScaleXZ);
    m_ShadowmapShader.setUniform1f("uScaleY", m_ScaleSettings.ScaleY);
    m_ShadowmapShader.setUniform1f("uMinT", m_ShadowSettings.MinT);
    m_ShadowmapShader.setUniform1f("uMaxT", m_ShadowSettings.MaxT);
    m_ShadowmapShader.setUniform1i("uSteps", m_ShadowSettings.Steps);
    m_ShadowmapShader.setUniform1f("uBias", m_ShadowSettings.Bias);
    
    BindGeometry();
    Draw();
}

void MapRenderer::Update(float theta, float phi) {
    if ((m_UpdateFlags & UpdateFlags::Height) != UpdateFlags::None)
        UpdateHeight();

    if ((m_UpdateFlags & UpdateFlags::Normal) != UpdateFlags::None)
        UpdateNormal();

    if ((m_UpdateFlags & UpdateFlags::Shadow) != UpdateFlags::None)
        UpdateShadow(theta, phi);

    m_UpdateFlags = UpdateFlags::None;
}

void MapRenderer::BindHeightmap(int id) {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, m_HeightmapTexture);
}

void MapRenderer::BindNormalmap(int id) {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, m_NormalmapTexture);
}

void MapRenderer::BindShadowmap(int id) {
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, m_ShadowTexture);
}

void MapRenderer::ImGuiTerrain(bool update_shadows) {
    HeightmapSettings temp_h = m_HeightSettings;
    ScaleSettings temp_s = m_ScaleSettings;

    ImGui::Begin("Terrain settings");
    ImGui::SliderFloat("Scale xz", &(temp_s.ScaleXZ), 0.0f, 400.0f);
    ImGui::SliderFloat("Scale y" , &(temp_s.ScaleY ), 0.0f, 100.0f);
    ImGui::SliderInt("Octaves", &temp_h.Octaves, 1, 16);
    ImGui::SliderFloat("Offset x", &temp_h.Offset[0], 0.0f, 20.0f);
    ImGui::SliderFloat("Offset y", &temp_h.Offset[1], 0.0f, 20.0f);
    ImGui::End();

    if (temp_h != m_HeightSettings) {
        m_HeightSettings = temp_h;
        m_UpdateFlags = m_UpdateFlags | UpdateFlags::Height;
        m_UpdateFlags = m_UpdateFlags | UpdateFlags::Normal;
        
        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | UpdateFlags::Shadow;
    }

    else if (temp_s != m_ScaleSettings) {
        m_ScaleSettings = temp_s;
        m_UpdateFlags = m_UpdateFlags | UpdateFlags::Normal;
        
        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | UpdateFlags::Shadow;
    }
}

void MapRenderer::ImGuiShadowmap(bool update_shadows) {
    ShadowmapSettings temp = m_ShadowSettings;

    ImGui::Begin("Shadowmap settings");
    ImGui::SliderFloat("Min t", &temp.MinT, 0.0, 0.5);
    ImGui::SliderFloat("Max t", &temp.MaxT, 0.0, 1.0);
    ImGui::SliderFloat("Mip bias", &temp.Bias, 0.0, 20.0);
    ImGui::SliderInt("Steps", &temp.Steps, 1, 64);
    ImGui::End();

    if (temp != m_ShadowSettings) {
        m_ShadowSettings = temp;
        if (update_shadows)
            m_UpdateFlags = m_UpdateFlags | UpdateFlags::Shadow;
    }
}

void MapRenderer::RequestShadowUpdate() {
    m_UpdateFlags = m_UpdateFlags | UpdateFlags::Shadow;
}

bool MapRenderer::GeometryShouldUpdate() {
    return (m_UpdateFlags & UpdateFlags::Height) != UpdateFlags::None ||
           (m_UpdateFlags & UpdateFlags::Normal) != UpdateFlags::None;
}

//Settings structs operator overloads:

bool operator==(const HeightmapSettings& lhs, const HeightmapSettings& rhs) {
    return (lhs.Octaves == rhs.Octaves)
        && (lhs.Offset[0] == rhs.Offset[0]) 
        && (lhs.Offset[1] == rhs.Offset[1]);
}

bool operator!=(const HeightmapSettings& lhs, const HeightmapSettings& rhs) {
    return !(lhs==rhs);
}

bool operator==(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return (lhs.ScaleXZ == rhs.ScaleXZ) && (lhs.ScaleY == rhs.ScaleY);
}

bool operator!=(const ScaleSettings& lhs, const ScaleSettings& rhs) {
    return !(lhs==rhs);
}

bool operator==(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs) {
    return (lhs.Steps == rhs.Steps) && (lhs.MinT == rhs.MinT)
        && (lhs.MaxT == rhs.MaxT) && (lhs.Bias == rhs.Bias);
}

bool operator!=(const ShadowmapSettings& lhs, const ShadowmapSettings& rhs) {
    return !(lhs==rhs);
}

UpdateFlags operator|(UpdateFlags x, UpdateFlags y) {
    return static_cast<UpdateFlags>(static_cast<int>(x) 
                                     | static_cast<int>(y));
}

UpdateFlags operator&(UpdateFlags x, UpdateFlags y) {
    return static_cast<UpdateFlags>(static_cast<int>(x) 
                                     & static_cast<int>(y));
}
