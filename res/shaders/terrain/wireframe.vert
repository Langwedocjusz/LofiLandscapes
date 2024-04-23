#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in uint aAuxData;

#define MAX_LEVELS 10
layout(std140, binding = 2) uniform ubo
{
    float QuadSizes[MAX_LEVELS];
};

uniform vec2 uPos;
uniform mat4 uMVP;

out vec3 EdgeColor;

#include "../common/clipmap.glsl"

void main() 
{
    bool trim_flag = false;
    uint edge_flag = 0, lvl = 0;

    UnpackAux(aAuxData, trim_flag, edge_flag, lvl);

    float quad_size = QuadSizes[lvl];

    vec2 pos2 = GetClipmapPos(aPos.xz, uPos, quad_size, trim_flag);
    vec3 pos3 = vec3(pos2.x, aPos.y, pos2.y);

    gl_Position = uMVP * vec4(pos3, 1.0);

    //Debug visualization of edge flags

    //Needs to match enum used to generate vertex data
    #define EDGE_FLAG_NONE 0
    #define EDGE_FLAG_HORIZONTAL 1
    #define EDGE_FLAG_VERTICAL 2

    if (edge_flag == EDGE_FLAG_NONE)
        EdgeColor = vec3(1.0);
    else if (edge_flag == EDGE_FLAG_HORIZONTAL)
        EdgeColor = vec3(0.0, 1.0, 0.0);
    else if (edge_flag == EDGE_FLAG_VERTICAL)
        EdgeColor = vec3(0.0, 0.0, 1.0);
}
