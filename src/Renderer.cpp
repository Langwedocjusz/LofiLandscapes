#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

Renderer::Renderer(unsigned int width, unsigned int height) 
    : m_WindowWidth(width), m_WindowHeight(height),
      m_Shader("res/shaders/test.vert", "res/shaders/test.frag")
{
    //Vertex data:
    for (int i=0; i<m_N*m_N; i++) {
        float origin[2] = {-m_L/2.0f, -m_L/2.0f};
        float offset[2] = {float(i%m_N)*(m_L/m_N), float(i/m_N)*(m_L/m_N)};

        m_VertexData.push_back(origin[0] + offset[0]);
        m_VertexData.push_back(0.0f);
        m_VertexData.push_back(origin[1] + offset[1]);
    }

    //Index data:
    for (int i=0; i<m_N*m_N; i++) {
        unsigned int ix = i % m_N;
        unsigned int iy = i/m_N;

        if (ix == m_N-1) continue;
        if (iy == m_N-1) continue;

        m_IndexData.push_back(i);
        m_IndexData.push_back(i+1);
        m_IndexData.push_back(i+m_N);
        
        m_IndexData.push_back(i+1);
        m_IndexData.push_back(i+1+m_N);
        m_IndexData.push_back(i+m_N);
    }

    //Buffers:
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_VertexData.size(), &m_VertexData[0], 
            GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_IndexData.size(), &m_IndexData[0],
            GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Renderer::~Renderer() {}

void Renderer::OnUpdate(float deltatime) {
    m_Camera.Update(deltatime);

    glm::mat4 proj = m_Camera.getProjMatrix(m_WindowWidth, m_WindowHeight);
    glm::mat4 view = m_Camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    m_MVP = proj * view * model;
}

void Renderer::OnRender() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_Shader.Bind();
    m_Shader.setUniformMatrix4fv("uMVP", m_MVP);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glDrawElements(GL_TRIANGLES, 6*m_N*m_N, GL_UNSIGNED_INT, 0);
}

void Renderer::OnImGuiRender() {
    ImGui::Begin("WE");
    ImGui::ColorEdit3("ClearColor", m_ClearColor);
    ImGui::End();
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {
    m_WindowWidth = width;
    m_WindowHeight = height;

    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    m_Camera.OnKeyPressed(keycode, repeat);

}

void Renderer::OnKeyReleased(int keycode) {
    m_Camera.OnKeyReleased(keycode);
}

void Renderer::OnMouseMoved(float x, float y) {
    m_Camera.OnMouseMoved(x, y, m_WindowWidth, m_WindowHeight);
}

void Renderer::RestartMouse() {
    m_Camera.setMouseInit(true);
}
