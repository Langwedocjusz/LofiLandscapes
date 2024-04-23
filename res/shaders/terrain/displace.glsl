#version 450 core

struct Vert {
    float PosX;
    float PosY;
    float PosZ;
    uint AuxData;
};

layout(std430, binding = 1) buffer vertexBuffer
{
    Vert verts[];
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

#define MAX_LEVELS 10
layout(std140, binding = 2) uniform ubo
{
    float QuadSizes[MAX_LEVELS];
};

uniform sampler2D heightmap;

uniform vec2 uPos;
uniform float uScaleY;
uniform float uScaleXZ;

//Needs to match enum used to generate vertex data
#define EDGE_FLAG_NONE 0
#define EDGE_FLAG_HORIZONTAL 1
#define EDGE_FLAG_VERTICAL 2

#include "../common/clipmap.glsl"

float getHeight(vec2 uv)
{
    return 0.5 * uScaleY * texture(heightmap, uv).r;
}

bool OffsetSamples(float pos, float quad_size, int id)
{
    bool offset_samples = (abs((mod(pos, 2.0*quad_size) - quad_size)) > 0.5*quad_size);

    if (id == 0) offset_samples = !offset_samples;

    return offset_samples;
}

void main() {
    uint i = gl_GlobalInvocationID.x;

    vec2 vert_pos = vec2(verts[i].PosX, verts[i].PosZ);

    bool trim_flag = false;
    uint edge_flag = 0, lvl = 0;

    UnpackAux(verts[i].AuxData, trim_flag, edge_flag, lvl);

    float quad_size = QuadSizes[lvl];

    //Base texture coordinate needs to be calculated after considering
    //possible movement of trim geometry
    int id = 0;
    ivec2 id2 = ivec2(0);
    vec2 hoffset = vec2(0);
    vec2 pos2 = GetClipmapPos(vert_pos, uPos, quad_size, trim_flag, id, id2, hoffset);

    vec2 uv = (2.0/uScaleXZ) * (pos2 + hoffset);
    uv = 0.5*uv + 0.5;

    #define CORRECT_SEAMS
    #ifdef CORRECT_SEAMS

    float height = 0.0;

    if (edge_flag == EDGE_FLAG_NONE)
    {
        height = getHeight(uv);
    }
    
    else 
    {
        //At this point the edge flag is either HORIZONTAL or VERTICAL
        //so we may store this in one bool
        bool horizontal = (edge_flag == EDGE_FLAG_HORIZONTAL);

        //Depending on the rotation of the trim mesh we may need to change 
        //the vertical/horizontal orientation of our seam-correction
        bool swap_axes = (trim_flag && (id == 1 || id == 2));

        if (swap_axes)
            horizontal = !horizontal;

        vec2 offset = horizontal
                    ? vec2(quad_size/uScaleXZ, 0.0)
                    : vec2(0.0, quad_size/uScaleXZ);

        bool offset_samples = horizontal
                            ? OffsetSamples(pos2.x, quad_size, id2.x)
                            : OffsetSamples(pos2.y, quad_size, id2.y);

        vec2 sample1 = uv, sample2 = uv;

        if(offset_samples)
        {
            sample1 -= offset;
            sample2 += offset;
        }

        height = 0.5*(getHeight(sample1)+getHeight(sample2));
    }

    #else
    float height = getHeight(uv);
    #endif

    verts[i].PosY = height;
}
