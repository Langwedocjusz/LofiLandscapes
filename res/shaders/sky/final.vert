#version 450 core

layout (location = 0) in vec3 aPos;

out vec2 uv;

void main() {
    uv = 0.5*aPos.xy + 0.5;

    //depth = 1.0 in normalized device coordinates
    gl_Position = vec4(aPos.x, aPos.y, 1.0, 1.0);
}
