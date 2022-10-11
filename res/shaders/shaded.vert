#version 450 core

layout (location = 0) in vec4 aPos;

out vec2 uv;

uniform float uL;
uniform mat4 uMVP;

uniform sampler2D tex;

void main() {
    uv = (2.0/uL) * aPos.xz;
    uv = 0.5*uv + 0.5;

    gl_Position = uMVP * aPos;
}
