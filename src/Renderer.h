#pragma once

class Renderer {
public:
    Renderer();
    ~Renderer();

    void OnUpdate();
    void OnImGuiUpdate();
private:
    float m_ClearColor[3] = {0.2f, 0.2f, 0.2f};
};
