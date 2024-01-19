#include "Texture.h"

#include "glad/glad.h"
#include "imgui.h"

#include <cstddef>

Texture::~Texture() {}

static void InitTex2D(uint32_t& id, Texture2DSpec spec) 
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, spec.InternalFormat,
        spec.ResolutionX, spec.ResolutionY, 0,
        spec.Format, spec.Type, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, spec.MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, spec.MagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, spec.Wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, spec.Wrap);

    if (spec.Wrap == GL_CLAMP_TO_BORDER)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, spec.Border);
}

static void InitTex3D(uint32_t& id, Texture3DSpec spec) 
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);

    glTexImage3D(GL_TEXTURE_3D, 0, spec.InternalFormat,
        spec.ResolutionX, spec.ResolutionY, spec.ResolutionZ,
        0, spec.Format, spec.Type, NULL);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, spec.MinFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, spec.MagFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, spec.Wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, spec.Wrap);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, spec.Wrap);

    if (spec.Wrap == GL_CLAMP_TO_BORDER)
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, spec.Border);
}

void Texture2D::Initialize(Texture2DSpec spec) 
{
    InitTex2D(m_ID, spec);
    m_Spec = spec;
}

void Texture2D::Bind(int id) const 
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture2D::BindImage(int id, int mip) const 
{
    int format = m_Spec.InternalFormat;

    glBindImageTexture(id, m_ID, mip, GL_FALSE, 0, GL_READ_WRITE, format);
}

void Texture2D::Resize(uint32_t width, uint32_t height)
{
    m_Spec.ResolutionX = width;
    m_Spec.ResolutionY = height;

    Bind();

    glTexImage2D(GL_TEXTURE_2D, 0, m_Spec.InternalFormat,
        m_Spec.ResolutionX, m_Spec.ResolutionY, 0,
        m_Spec.Format, m_Spec.Type, NULL);
}

void Texture2D::DrawToImGui(float width, float height) 
{
    ImGui::Image((void*)(intptr_t)m_ID, ImVec2(width, height));
}

void TextureArray::Initialize(Texture2DSpec spec, int layers) 
{

    auto log2 = [](int value) {
        int copy = value, result = 0;
        while (copy >>= 1) ++result;
        return result;
    };

    int mips = log2(spec.ResolutionX);

    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);

    //Storage instead of Image is crucial since TextureViews REQUIRE immutable storage
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mips, spec.InternalFormat,
        spec.ResolutionX, spec.ResolutionY, layers);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, spec.MinFilter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, spec.MagFilter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, spec.Wrap);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, spec.Wrap);

    if (spec.Wrap == GL_CLAMP_TO_BORDER)
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, spec.Border);

    for (int i = 0; i < layers; i++) {
        m_TextureViews.push_back(0);

        glGenTextures(1, &m_TextureViews[i]);

        glTextureView(m_TextureViews[i], GL_TEXTURE_2D, m_ID,
            spec.InternalFormat, 0, 1, i, 1);
    }

    m_Spec = spec;
    m_Layers = layers;
}

void TextureArray::Bind(int id) const 
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);
}

void TextureArray::BindLayer(int id, int layer) const 
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, m_TextureViews[layer]);
}

void TextureArray::BindImage(int id, int layer, int mip) const 
{
    int format = m_Spec.InternalFormat;

    glBindImageTexture(id, m_ID, mip, GL_FALSE, layer, GL_READ_WRITE, format);
}

void Texture3D::Initialize(Texture3DSpec spec) 
{
    InitTex3D(m_ID, spec);
    m_Spec = spec;
}

void Texture3D::Bind(int id) const 
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_3D, m_ID);
}

void Texture3D::BindImage(int id, int mip) const 
{
    int format = m_Spec.InternalFormat;

    glBindImageTexture(id, m_ID, mip, GL_TRUE, 0, GL_READ_WRITE, format);
}

void Cubemap::Initialize(CubemapSpec spec) 
{
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, spec.InternalFormat,
            spec.Resolution, spec.Resolution, 0,
            spec.Format, spec.Type, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, spec.MinFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, spec.MagFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_Spec = spec;
}

void Cubemap::Bind(int id) const 
{
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
}

void Cubemap::BindImage(int id, int mip) const 
{
    int format = m_Spec.InternalFormat;

    glBindImageTexture(id, m_ID, mip, GL_TRUE, 0, GL_READ_WRITE, format);
}