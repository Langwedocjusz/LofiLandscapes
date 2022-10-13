#include "MapRenderer.h"

#include "glad/glad.h"

#include <iostream>

bool operator==(const HeightmapParams& lhs, const HeightmapParams& rhs) {
    return (lhs.Octaves == rhs.Octaves) && (lhs.Offset[0] == rhs.Offset[0]) && (lhs.Offset[1] == rhs.Offset[1]);
}

bool operator!=(const HeightmapParams& lhs, const HeightmapParams& rhs) {
    return !(lhs==rhs);
}

MapRenderer::MapRenderer()
    : m_HeightmapShader("res/shaders/quad.vert", "res/shaders/height.frag"),
      m_NormalmapShader("res/shaders/quad.vert", "res/shaders/normal.frag")
{
    //Quad (GL Buffers only):
    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);
    glGenBuffers(1, &m_QuadEBO);

    glBindVertexArray(m_QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_QuadVertexData), &m_QuadVertexData, 
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_QuadIndexData), 
                    &m_QuadIndexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Heightmap
    //Framebuffer
    const int tex_res = m_HeightmapParams.Resolution;
    
    glGenFramebuffers(1, &m_HeightmapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_HeightmapFBO);

    //Texture
    glGenTextures(1, &m_HeightmapTexture);
    glBindTexture(GL_TEXTURE_2D, m_HeightmapTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_res, tex_res, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_HeightmapTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FRAMEBUFFER NOT READY \n";

    //Normal map:
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    //Attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_NormalmapTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "FRAMEBUFFER NOT READY \n";
    
}

MapRenderer::~MapRenderer() {

}

void MapRenderer::BindGeometry() {
    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadEBO);
}

void MapRenderer::Draw() {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void MapRenderer::Update() {
    glViewport(0, 0, m_HeightmapParams.Resolution, 
                     m_HeightmapParams.Resolution);
    
    //Render to heightmap
    glBindFramebuffer(GL_FRAMEBUFFER, m_HeightmapFBO);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_HeightmapShader.Bind();
    m_HeightmapShader.setUniform1i("uOctaves", m_HeightmapParams.Octaves);
    m_HeightmapShader.setUniform2f("uOffset", m_HeightmapParams.Offset[0],
                                              m_HeightmapParams.Offset[1]);
    BindGeometry();
    Draw();

    //Render to normal map:
    glBindTexture(GL_TEXTURE_2D, m_HeightmapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, m_NormalmapFBO);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_NormalmapShader.Bind();
    BindGeometry();
    Draw();
}

void MapRenderer::BindHeightmap() {
    glBindTexture(GL_TEXTURE_2D, m_HeightmapTexture);
}

void MapRenderer::BindNormalmap() {
    glBindTexture(GL_TEXTURE_2D, m_NormalmapTexture);
}

