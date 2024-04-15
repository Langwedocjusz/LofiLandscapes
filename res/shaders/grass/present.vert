#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aQuadSize;

layout (location = 3) in float aTrimFlag;

uniform float uScaleXZ;
uniform mat4 uMVP;

uniform vec3 uPos;

uniform float uGrassHeight;

out vec2 world_uv;

out vec3 frag_pos;

#include "../common/clipmap_move.glsl"

void main() {
    vec2 pos2 = GetClipmapPos(aPos.xz, uPos.xz, aQuadSize, aTrimFlag);
    vec3 pos3 = vec3(pos2.x, aPos.y + uGrassHeight, pos2.y);

    world_uv = (2.0/uScaleXZ) * pos2;
    world_uv = 0.5*world_uv + 0.5;
    
    frag_pos = pos3;

    gl_Position = uMVP * vec4(pos3, 1.0);
}
