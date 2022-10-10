#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();

    void OnUpdate();
    void OnImGuiUpdate();

    void OnWindowResize(unsigned int width, unsigned int height);
    void OnKeyPressed(int keycode, bool repeat);
    void OnKeyReleased(int keycode);
    void OnMouseMoved(float x, float y);
private:
    bool m_ShowMenu = false;
    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};
};
