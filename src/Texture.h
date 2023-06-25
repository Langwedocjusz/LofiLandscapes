#pragma once

#include <vector>

class Texture {
public:
    virtual ~Texture() = 0;
};

struct Texture2DSpec {
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

class Texture2D : public Texture {
public:
    void Initialize(Texture2DSpec spec);
    void Bind(int id = 0) const;
    void BindImage(int id, int mip) const;
    void AttachToFramebuffer();

    void DrawToImGui(float width, float height);

    const Texture2DSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_ID;
    Texture2DSpec m_Spec;
};

class TextureArray : public Texture {
public:

    void Initialize(Texture2DSpec spec, int layers);

    void Bind(int id = 0) const;
    void BindLayer(int id, int layer) const;
    void BindImage(int id, int layer, int mip) const;

    const Texture2DSpec& getSpec() { return m_Spec; }
    int getLayers() { return m_Layers; }

private:
    unsigned int m_ID;
    int m_Layers;
    Texture2DSpec m_Spec;

    std::vector<unsigned int> m_TextureViews;
};

class FramebufferTexture{
public:
    FramebufferTexture();
    ~FramebufferTexture();

    void Initialize(Texture2DSpec spec);
    void BindFBO();
    void BindTex(int id = 0);

    const Texture2DSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_FBO, m_ID;
    Texture2DSpec m_Spec;
};

struct Texture3DSpec {
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

class Texture3D : public Texture {
public:
    void Initialize(Texture3DSpec spec);
    void Bind(int id = 0) const;
    void BindImage(int id, int mip) const;

    const Texture3DSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_ID;
    Texture3DSpec m_Spec;
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

class Cubemap : public Texture {
public:
    void Initialize(CubemapSpec spec);
    void Bind(int id = 0) const;
    void BindImage(int id, int mip) const;

    const CubemapSpec& getSpec() { return m_Spec; }
private:
    unsigned int m_ID;
    CubemapSpec m_Spec;
};