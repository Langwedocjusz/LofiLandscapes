#include "Framebuffer.h"

#include "glad/glad.h"

Framebuffer::Framebuffer(ResourceManager& manager)
    : m_ResourceManager(manager)
{
    m_ColorAttachment = m_ResourceManager.RequestTexture2D();
}

Framebuffer::~Framebuffer() 
{
    glDeleteFramebuffers(1, &m_FBO);
}

void Framebuffer::Initialize(Texture2DSpec color_spec)
{
    //Initialize FrameBufferObject:
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    //Initialize RenderBufferObject containing depth and tencil attachments
    glGenRenderbuffers(1, &m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color_spec.ResolutionX, color_spec.ResolutionY);

    //Attach RBO to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    m_ColorAttachment->Initialize(color_spec);

    //Attach color texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment->m_ID, 0);

    //Reset framebuffer to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::BindFBO() const 
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
}

void Framebuffer::BindColorTex(int id) const 
{
    m_ColorAttachment->Bind(id);
}

void Framebuffer::BindColorImage(int id, int mip) const 
{
    m_ColorAttachment->BindImage(id, mip);
}

void Framebuffer::Resize(uint32_t width, uint32_t height)
{
    m_ColorAttachment->Resize(width, height);

    //Resize renderbuffer
    const int res_x = m_ColorAttachment->getResolutionX();
    const int res_y = m_ColorAttachment->getResolutionY();

    glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, res_x, res_y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}