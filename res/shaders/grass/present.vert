#version 450 core

layout (location = 0) in vec4 aPos;

uniform float uL;
uniform mat4 uMVP;

uniform vec3 uPos;

uniform float uGrassHeight;

out vec2 world_uv;

out vec3 frag_pos;

void main() {
    vec2 hoffset = uPos.xz - mod(uPos.xz, aPos.w);

    world_uv = (2.0/uL) * (aPos.xz + hoffset);
    world_uv = 0.5*world_uv + 0.5;
    
    frag_pos = aPos.xyz + vec3(hoffset.x, uGrassHeight, hoffset.y);

    gl_Position = uMVP * vec4(aPos.xyz + vec3(hoffset.x, uGrassHeight, hoffset.y), 1.0);
}
