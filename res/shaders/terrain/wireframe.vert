#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aQuadSize;
layout (location = 2) in float aEdgeFlag;
layout (location = 3) in float aTrimFlag;

uniform mat4 uMVP;

uniform vec2 uPos;

out vec3 EdgeColor;

#include "../common/clipmap_move.glsl"

void main() 
{
    vec2 pos2 = GetClipmapPos(aPos.xz, uPos, aQuadSize, aTrimFlag);
    vec3 pos3 = vec3(pos2.x, aPos.y, pos2.y);

    gl_Position = uMVP * vec4(pos3, 1.0);

    //Debug visualization of edge flags

    //Needs to match enum used to generate vertex data
    #define EDGE_FLAG_NONE 0.0
    #define EDGE_FLAG_HORIZONTAL 1.0
    #define EDGE_FLAG_VERTICAL 2.0

    if (aEdgeFlag == EDGE_FLAG_NONE)
        EdgeColor = vec3(1.0);
    else if (aEdgeFlag == EDGE_FLAG_HORIZONTAL)
        EdgeColor = vec3(0.0, 1.0, 0.0);
    else if (aEdgeFlag == EDGE_FLAG_VERTICAL)
        EdgeColor = vec3(0.0, 0.0, 1.0);
}
