#pragma once

#include <vector>
#include <cstdint>

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
    friend class Framebuffer;

    void Initialize(Texture2DSpec spec);
    void Bind(int id = 0) const;
    void BindImage(int id, int mip) const;

    void Resize(uint32_t width, uint32_t height);

    void DrawToImGui(float width, float height);

    int getResolutionX() const { return m_Spec.ResolutionX; }
    int getResolutionY() const { return m_Spec.ResolutionY; }

    const Texture2DSpec& getSpec() const { return m_Spec; }
private:
    uint32_t m_ID;
    Texture2DSpec m_Spec;
};

class TextureArray : public Texture {
public:
    void Initialize(Texture2DSpec spec, int layers);

    void Bind(int id = 0) const;
    void BindLayer(int id, int layer) const;
    void BindImage(int id, int layer, int mip) const;

    int getResolutionX() const { return m_Spec.ResolutionX; }
    int getResolutionY() const { return m_Spec.ResolutionY; }
    uint32_t getLayers() const { return m_Layers; }

    const Texture2DSpec& getSpec() const { return m_Spec; }

private:
    uint32_t m_ID;
    uint32_t m_Layers;
    Texture2DSpec m_Spec;

    std::vector<uint32_t> m_TextureViews;
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

    int getResolutionX() const { return m_Spec.ResolutionX; }
    int getResolutionY() const { return m_Spec.ResolutionY; }
    int getResolutionZ() const { return m_Spec.ResolutionZ; }

    const Texture3DSpec& getSpec() { return m_Spec; }
private:
    uint32_t m_ID;
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

    int getResolution() const { return m_Spec.Resolution; }

    const CubemapSpec& getSpec() const { return m_Spec; }
private:
    uint32_t m_ID;
    CubemapSpec m_Spec;
};