#pragma once

#include <vector>

class Quad{
public:
    Quad();
    ~Quad();

    void Draw();
private:
    unsigned int m_VAO, m_VBO, m_EBO;
    
    float m_VertexData[12] = {-1.0f, 1.0f, 0.5f,
                               1.0f, 1.0f, 0.5f,
                               1.0f,-1.0f, 0.5f,
                              -1.0f,-1.0f, 0.5f };
    
    unsigned int m_IndexData[6] = {0,1,3, 1,2,3};
};

struct TextureSpec{
    int ResolutionX;
    int ResolutionY;
    //Assuming trivial conversion GLenum->int
    int InternalFormat;
    int Format;
    int Type;
    int MagFilter;
    int MinFilter;
    int Wrap;
    float Border[4];
};

class Texture{
public:
    Texture();
    ~Texture();

    void Initialize(TextureSpec spec);
    void Bind(int id=0) const;
    void BindImage(int id, int mip) const;
    void AttachToFramebuffer();

    const TextureSpec& getSpec() {return m_Spec;}
private:
    unsigned int m_Texture;
    TextureSpec m_Spec;
};

class TextureArray {
public:
    TextureArray();
    ~TextureArray();

    void Initialize(TextureSpec spec, int layers);

    void Bind(int id = 0) const;
    void BindLayer(int id, int layer) const;
    void BindImage(int id, int layer, int mip) const;

    const TextureSpec& getSpec() { return m_Spec; }
    int getLayers() { return m_Layers; }

private:
    unsigned int m_Texture;
    int m_Layers;
    TextureSpec m_Spec;

    std::vector<unsigned int> m_TextureViews;
};

class FramebufferTexture{
public:
    FramebufferTexture();
    ~FramebufferTexture();

    void Initialize(TextureSpec spec);
    void BindFBO();
    void BindTex(int id=0);
    
    const TextureSpec& getSpec() {return m_Spec;}
private:
    unsigned int m_FBO, m_Texture;
    TextureSpec m_Spec;
};

struct Texture3dSpec {
    int ResolutionX;
    int ResolutionY;
    int ResolutionZ;
    //Assuming trivial conversion GLenum->int
    int InternalFormat;
    int Format;
    int Type;
    int MagFilter;
    int MinFilter;
    int Wrap;
    float Border[4];
};

class Texture3d {
public:
    Texture3d();
    ~Texture3d();

    void Initialize(Texture3dSpec spec);
    void Bind(int id = 0) const;
    void BindImage(int id, int mip) const;

    const Texture3dSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_Texture;
    Texture3dSpec m_Spec;
};

struct CubemapSpec {
    //Represents one slice, assumed square shape
    int Resolution;
    //Assuming trivial conversion GLenum->int
    int InternalFormat;
    int Format;
    int Type;
    int MagFilter;
    int MinFilter;
};

class Cubemap {
public:
    Cubemap();
    ~Cubemap();

    void Initialize(CubemapSpec spec);
    void Bind(int id=0) const;
    void BindImage(int id, int mip) const;

    const CubemapSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_Texture;
    CubemapSpec m_Spec;
};