#pragma once

class Quad{
public:
    Quad();
    ~Quad();

    void Draw();
private:
    unsigned int m_VAO, m_VBO, m_EBO;
    
    float m_VertexData[12] = {-1.0f, 1.0f, 1.0f,
                               1.0f, 1.0f, 1.0f,
                               1.0f,-1.0f, 1.0f,
                              -1.0f,-1.0f, 1.0f };
    
    unsigned int m_IndexData[6] = {0,1,3, 1,2,3};
};

struct TextureSpec{
    int Resolution;
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
    void Bind(int id=0);
    void BindImage(int id, int mip);

    const TextureSpec& getSpec() {return m_Spec;}
private:
    unsigned int m_Texture;
    TextureSpec m_Spec;
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
