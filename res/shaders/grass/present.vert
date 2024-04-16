#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int aTrimFlag;

uniform float uScaleXZ;
uniform mat4 uMVP;

uniform vec3 uPos;

uniform float uGrassHeight;

out vec2 world_uv;

out vec3 frag_pos;

//#define MAX_DRAWABLE_ID 132
//layout(std430, binding = 2) buffer ssbo
//{
//    float QuadSizes[MAX_DRAWABLE_ID];
//    float TrimFlags[MAX_DRAWABLE_ID];
//};

#define MAX_LEVELS 10
layout(std140, binding = 2) uniform ubo
{
    float QuadSizes[MAX_LEVELS];
};

uniform int uDrawableID;

#include "../common/clipmap_move.glsl"

void main() {
    //float quad_size = QuadSizes[uDrawableID];
    //float trim_flag = TrimFlags[uDrawableID];
    float quad_size = QuadSizes[abs(uDrawableID)-1];
    float trim_flag = float(uDrawableID < 0.0);

    vec2 pos2 = GetClipmapPos(aPos.xz, uPos.xz, quad_size, trim_flag);
    vec3 pos3 = vec3(pos2.x, aPos.y + uGrassHeight, pos2.y);

    world_uv = (2.0/uScaleXZ) * pos2;
    world_uv = 0.5*world_uv + 0.5;
    
    frag_pos = pos3;

    gl_Position = uMVP * vec4(pos3, 1.0);
}
