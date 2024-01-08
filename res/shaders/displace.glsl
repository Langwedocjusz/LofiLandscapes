#version 450 core

struct Vert {
    vec4 Pos; //includes 1 float (4 bytes) of padding
    float QuadSize;
    float EdgeFlag;
    float TrimFlag;
    float Padding;
};

layout(std430, binding = 1) buffer vertexBuffer
{
    Vert verts[];
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
uniform vec2 uPos;

//Needs to match enum used to generate vertex data
#define EDGE_FLAG_NONE 0.0
#define EDGE_FLAG_HORIZONTAL 1.0
#define EDGE_FLAG_VERTICAL 2.0

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

    float quad_size = verts[i].QuadSize;

    vec2 hoffset = uPos - mod(uPos, quad_size);

    vec2 pos2 = verts[i].Pos.xz;

    //Trim geometry needs to be mirrored along x/z axis
    //during each move. We simulate this with rotation matrices, 
    //since we cannot change orientation as we are using face culling.
    mat2 rotations[4] = mat2[4](
        mat2(1.0, 0.0, 0.0, 1.0),
        mat2(0.0, 1.0, -1.0, 0.0),
        mat2(0.0, -1.0, 1.0, 0.0),
        mat2(-1.0, 0.0, 0.0, -1.0)
    );

    ivec2 id2 = ivec2(hoffset/quad_size);
    id2.x = id2.x % 2;
    id2.y = id2.y % 2;

    int id = id2.x + 2*id2.y;

    bool is_trim_vertex = (verts[i].TrimFlag == 1.0);

    if(is_trim_vertex)
    {
        pos2 = rotations[id] * pos2;
        pos2 += quad_size * vec2(1-id2.x, 1-id2.y);
    }

    //Base texture coordinate needs to be calculated after considering
    //possible movement of trim geometry
    vec2 uv = (2.0/uScaleXZ) * (pos2 + hoffset);
    uv = 0.5*uv + 0.5;

    #define CORRECT_SEAMS
    #ifdef CORRECT_SEAMS

    float height = 0.0;

    if (verts[i].EdgeFlag == EDGE_FLAG_NONE)
    {
        height = getHeight(uv);
    }
    
    else 
    {
        //At this point the edge flag is either HORIZONTAL or VERTICAL
        //so we may store this in one bool
        bool horizontal = (verts[i].EdgeFlag == EDGE_FLAG_HORIZONTAL);

        //Depending on the rotation of the trim mesh we may need to change 
        //the vertical/horizontal orientation of our seam-correction
        bool swap_axes = (is_trim_vertex && (id == 1 || id == 2));

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

    verts[i].Pos.y = height;
}
