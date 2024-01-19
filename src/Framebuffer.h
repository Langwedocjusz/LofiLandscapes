#pragma once

#include "Texture.h"
#include "ResourceManager.h"

class Framebuffer {
public:
    Framebuffer(ResourceManager& manager);

    ~Framebuffer();

    void Initialize(Texture2DSpec color_spec);

    void BindFBO() const;
    void BindColorTex(int id = 0) const;
    void BindColorImage(int id, int mip) const;

    void Resize(uint32_t width, uint32_t height);

    int getResolutionX() const { return m_ColorAttachment->m_Spec.ResolutionX; }
    int getResolutionY() const { return m_ColorAttachment->m_Spec.ResolutionY; }

    const Texture2DSpec& getSpec() const { return m_ColorAttachment->m_Spec; }
private:
    uint32_t m_FBO, m_RBO;

    std::shared_ptr<Texture2D> m_ColorAttachment;

    ResourceManager& m_ResourceManager;
};