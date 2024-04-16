#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int aEdgeFlag;

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

uniform mat4 uMVP;
uniform vec2 uPos;

out vec3 EdgeColor;

#include "../common/clipmap_move.glsl"

void main() 
{
    //float quad_size = QuadSizes[uDrawableID];
    //float trim_flag = TrimFlags[uDrawableID];
    float quad_size = QuadSizes[abs(uDrawableID)-1];
    float trim_flag = float(uDrawableID < 0.0);

    vec2 pos2 = GetClipmapPos(aPos.xz, uPos, quad_size, trim_flag);
    vec3 pos3 = vec3(pos2.x, aPos.y, pos2.y);

    gl_Position = uMVP * vec4(pos3, 1.0);

    //Debug visualization of edge flags

    //Needs to match enum used to generate vertex data
    #define EDGE_FLAG_NONE 0
    #define EDGE_FLAG_HORIZONTAL 1
    #define EDGE_FLAG_VERTICAL 2

    if (aEdgeFlag == EDGE_FLAG_NONE)
        EdgeColor = vec3(1.0);
    else if (aEdgeFlag == EDGE_FLAG_HORIZONTAL)
        EdgeColor = vec3(0.0, 1.0, 0.0);
    else if (aEdgeFlag == EDGE_FLAG_VERTICAL)
        EdgeColor = vec3(0.0, 0.0, 1.0);
}
