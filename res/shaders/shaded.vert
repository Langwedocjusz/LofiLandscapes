#version 450 core

layout (location = 0) in vec4 aPos;

out vec2 uv;

uniform float uL;
uniform vec2 uPos;
uniform mat4 uMVP;

uniform sampler2D tex;

void main() {
    vec2 hoffset = uPos - mod(uPos, aPos.w);

    uv = (2.0/uL) * (aPos.xz + hoffset);
    uv = 0.5*uv + 0.5;

    gl_Position = uMVP * vec4(aPos.xyz + vec3(hoffset.x, 0.0, hoffset.y), 1.0);
}
