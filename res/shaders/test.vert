#version 450 core

layout (location = 0) in vec3 aPos;

out vec2 uv;

uniform float uL;
uniform mat4 uMVP;

uniform sampler2D tex;

void main() {
    uv = (2.0/uL) * aPos.xz;
    uv = 0.5*uv + 0.5;

    float height = 0.5*uL * texture(tex, uv).a;

    gl_Position = uMVP * vec4(aPos + vec3(0.0, height, 0.0), 1.0);
}
