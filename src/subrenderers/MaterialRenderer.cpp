#include "MaterialRenderer.h"

#include "glad/glad.h"

#include "imgui.h"
#include "ImGuiUtils.h"

#include <iostream>

void ClearColorTexture(Texture& tex, float r, float g, float b, float a) {
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    tex.Bind();
    tex.AttachToFramebuffer();

    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
}

MaterialRenderer::MaterialRenderer()
    : m_NormalShader("res/shaders/materials/normal.glsl")
{
    //=====Initialize the textures:
    TextureSpec height_spec = TextureSpec{
        512, GL_R16F, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Height.Initialize(height_spec);
    ClearColorTexture(m_Height, 0.0f, 0.0f, 0.0f, 0.0f);

    TextureSpec spec = TextureSpec{
        512, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
        GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,
        GL_REPEAT,
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    m_Albedo.Initialize(spec);
    ClearColorTexture(m_Albedo, 1.0f, 1.0f, 1.0f, 1.0f);
    m_Albedo.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    m_Normal.Initialize(spec);
    ClearColorTexture(m_Normal, 0.5f, 1.0f, 0.5f, 1.0f);
    m_Normal.Bind();
    glGenerateMipmap(GL_TEXTURE_2D);


    //=====Initialize material editors:
    std::vector<std::string> labels{ "Average", "Add", "Subtract" };
    
    //Heightmap
    m_HeightEditor.RegisterShader("Const Value", "res/shaders/materials/const_val.glsl");
    m_HeightEditor.AttachSliderFloat("Const Value", "uValue", "Value", 0.0, 1.0, 0.0);
    m_HeightEditor.AddProcedureInstance("Const Value");

    m_HeightEditor.RegisterShader("FBM", "res/shaders/materials/fbm.glsl");
    m_HeightEditor.AttachConstInt("FBM", "uResolution", m_Height.getSpec().Resolution);
    m_HeightEditor.AttachSliderInt("FBM", "uOctaves", "Octaves", 1, 16, 8);
    m_HeightEditor.AttachSliderInt("FBM", "uScale", "Scale", 
                                     0, 100, 1);
    m_HeightEditor.AttachSliderFloat("FBM", "uRoughness", "Roughness", 0.0, 1.0, 0.5);
    m_HeightEditor.AttachGLEnum("FBM", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("FBM", "uWeight", "Weight", 0.0, 1.0, 1.0);


    m_HeightEditor.RegisterShader("Voronoi", "res/shaders/materials/voronoi.glsl");
    m_HeightEditor.AttachConstInt("Voronoi", "uResolution", m_Height.getSpec().Resolution);
    m_HeightEditor.AttachSliderInt("Voronoi", "uScale", "Scale", 0, 100, 1);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uRandomness", "Randomness", 0.0, 1.0, 1.0);
    
    std::vector<std::string> voro_types{"F1", "F2", "F2_F1"};
    m_HeightEditor.AttachGLEnum("Voronoi", "uVoronoiType", "Type", voro_types);
    
    m_HeightEditor.AttachGLEnum("Voronoi", "uBlendMode", "Blend Mode", labels);
    m_HeightEditor.AttachSliderFloat("Voronoi", "uWeight", "Weight", 0.0, 1.0, 1.0);

    //Albedo
    m_AlbedoEditor.RegisterShader("Color Ramp", "res/shaders/materials/color_ramp.glsl");
    m_AlbedoEditor.AttachConstInt("Color Ramp", "uResolution", m_Albedo.getSpec().Resolution);
    m_AlbedoEditor.AttachSliderFloat("Color Ramp", "uEdge1", "Edge 1",
                                     0.0f, 1.0f, 0.0f);
    m_AlbedoEditor.AttachSliderFloat("Color Ramp", "uEdge2", "Edge 2",
                                     0.0f, 1.0f, 1.0f);
    m_AlbedoEditor.AttachColorEdit3("Color Ramp", "uCol1", "Color 1",
                                    glm::vec3(0.0f));
    m_AlbedoEditor.AttachColorEdit3("Color Ramp", "uCol2", "Color 2",
                                    glm::vec3(1.0f));
}

MaterialRenderer::~MaterialRenderer() {

}

void MaterialRenderer::Update() { 

    //Draw to heightmap:
    if ((m_UpdateFlags & MaterialUpdateFlags::Height) 
                      != MaterialUpdateFlags::None)
    {
        const int res = m_Height.getSpec().Resolution;
        
        m_Height.BindImage(0, 0);
        m_HeightEditor.OnDispatch(res);
    }

    //Draw to normal:
    if ((m_UpdateFlags & MaterialUpdateFlags::Normal) 
                      != MaterialUpdateFlags::None)
    {
        const int res = m_Normal.getSpec().Resolution;
        m_Height.Bind();
        
        m_Normal.BindImage(0, 0);

        m_NormalShader.Bind();
        m_NormalShader.setUniform1i("uResolution", res);
        m_NormalShader.setUniform1f("uAOStrength", 1.0f/m_AOStrength);
        m_NormalShader.setUniform1f("uAOSpread", m_AOSpread);
        m_NormalShader.setUniform1f("uAOContrast", m_AOContrast);

        //Magic number "32" needs to be the same as local size
        //declared in the compute shader files
        glDispatchCompute(res/32, res/32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    
        m_Normal.Bind();
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    //Draw to albedo:
    if ((m_UpdateFlags & MaterialUpdateFlags::Albedo) 
                      != MaterialUpdateFlags::None)
    {
        const int res = m_Albedo.getSpec().Resolution;
        m_Height.Bind();

        m_Albedo.BindImage(0, 0);
        m_AlbedoEditor.OnDispatch(res);
    
        m_Albedo.Bind();
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_UpdateFlags = MaterialUpdateFlags::None;
}

void MaterialRenderer::OnImGui(bool& open) {
    ImGui::Begin("Material editor", &open);
    
    ImGui::Text("Heightmap procedures:");

    if (m_HeightEditor.OnImGui()) {
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Height;
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Normal;
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Albedo;
    }

    if (ImGuiUtils::Button("Add heightmap procedure")) {        
        ImGui::OpenPopup("Choose procedure (height)");
    }

    if (ImGui::BeginPopupModal("Choose procedure (height)")) {

        if (ImGui::Button("Const Value")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("Const Value");
        }

        if (ImGui::Button("FBM")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("FBM");
        }     

        if (ImGui::Button("Voronoi")) {
            ImGui::CloseCurrentPopup();
            m_HeightEditor.AddProcedureInstance("Voronoi");
        }

        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Height;
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Normal;
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Albedo;

        ImGui::EndPopup();
    }

    ImGui::Separator();

    ImGui::Text("Normal/AO map settings:");

    float tmp_str = m_AOStrength, tmp_spr = m_AOSpread, tmp_c = m_AOContrast;  

    ImGuiUtils::SliderFloat("AO Strength", &tmp_str, 0.01, 1.0);
    ImGuiUtils::SliderFloat("AO Spread", &tmp_spr, 1.0, 10.0);
    ImGuiUtils::SliderFloat("AO Contrast", &tmp_c, 0.1, 5.0);

    if (tmp_str != m_AOStrength || tmp_spr != m_AOSpread || tmp_c != m_AOContrast) {
        m_AOStrength = tmp_str;
        m_AOSpread = tmp_spr;
        m_AOContrast = tmp_c;
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Normal;
    }

    ImGui::Separator();

    ImGui::Text("Albedo procedures:");
    
    if (m_AlbedoEditor.OnImGui()) {
        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Albedo;
    }

    if (ImGuiUtils::Button("Add albedo procedure")) {
        ImGui::OpenPopup("Choose procedure (albedo)");
    }

    if (ImGui::BeginPopupModal("Choose procedure (albedo)")) {

        if (ImGui::Button("Color Ramp")) {
            ImGui::CloseCurrentPopup();
            m_AlbedoEditor.AddProcedureInstance("Color Ramp");
        }

        m_UpdateFlags = m_UpdateFlags | MaterialUpdateFlags::Albedo;

        ImGui::EndPopup();
    }

    ImGui::End();

    Update();
}

void MaterialRenderer::BindAlbedo(int id) {
    m_Albedo.Bind(id);
}

void MaterialRenderer::BindNormal(int id) {
    m_Normal.Bind(id);
}

//Operator overloads for update flags:

MaterialUpdateFlags operator|(MaterialUpdateFlags x, MaterialUpdateFlags y) {
    return static_cast<MaterialUpdateFlags>(
        static_cast<int>(x) | static_cast<int>(y)
    );
}

MaterialUpdateFlags operator&(MaterialUpdateFlags x, MaterialUpdateFlags y) {
    return static_cast<MaterialUpdateFlags>(
        static_cast<int>(x) & static_cast<int>(y)
    );
}
