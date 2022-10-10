#include "Renderer.h"
#include "Keycodes.h"

#include "glad/glad.h"

#include "imgui.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::OnUpdate() {
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::OnImGuiUpdate() {
    if (!m_ShowMenu) return;

    ImGui::Begin("WE");
    ImGui::ColorEdit3("ClearColor", m_ClearColor);
    ImGui::End();
}

void Renderer::OnWindowResize(unsigned int width, unsigned int height) {}

void Renderer::OnKeyPressed(int keycode, bool repeat) {
    if (keycode == LOFI_KEY_ESCAPE && !repeat)
        m_ShowMenu = (!m_ShowMenu);
}

void Renderer::OnKeyReleased(int keycode) {}
void Renderer::OnMouseMoved(float x, float y) {}
