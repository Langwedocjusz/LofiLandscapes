#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();

    void OnUpdate();
    void OnImGuiUpdate();

    void OnKeyPressed(int keycode, bool repeat);
private:
    bool m_ShowMenu = false;
    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};
};
