#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in uint aAuxData;

uniform vec3 uPos;
uniform mat4 uMVP;
uniform float uScaleXZ;
uniform float uGrassHeight;

out vec2 world_uv;
out vec3 frag_pos;

#define MAX_LEVELS 10
layout(std140, binding = 2) uniform ubo
{
    float QuadSizes[MAX_LEVELS];
};

#include "../common/clipmap.glsl"

void main() {
    bool trim_flag = false;
    uint edge_flag = 0, lvl = 0;

    UnpackAux(aAuxData, trim_flag, edge_flag, lvl);

    float quad_size = QuadSizes[lvl];

    vec2 pos2 = GetClipmapPos(aPos.xz, uPos.xz, quad_size, trim_flag);
    vec3 pos3 = vec3(pos2.x, aPos.y + uGrassHeight, pos2.y);

    world_uv = (2.0/uScaleXZ) * pos2;
    world_uv = 0.5*world_uv + 0.5;
    
    frag_pos = pos3;

    gl_Position = uMVP * vec4(pos3, 1.0);
}
