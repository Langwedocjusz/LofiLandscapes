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
        
    else if (verts[i].EdgeFlag == EDGE_FLAG_HORIZONTAL)
    {
        vec2 sample1 = uv, sample2 = uv;

        vec2 offset = vec2(quad_size/uScaleXZ, 0.0);

        if (is_trim_vertex)
            offset = rotations[id] * offset;

        bool offset_samples = is_trim_vertex
                            ? int(verts[i].Pos.x/quad_size) % 2 != 1
                            : int(verts[i].Pos.x/quad_size) % 2 != id2.x;

        if(offset_samples)
        {
            sample1 -= offset;
            sample2 += offset;
        }

        height = 0.5*(getHeight(sample1)+getHeight(sample2));
    }

    else if (verts[i].EdgeFlag == EDGE_FLAG_VERTICAL)
    {
        vec2 sample1 = uv, sample2 = uv;

        vec2 offset = vec2(0.0, quad_size/uScaleXZ);

        if (is_trim_vertex)
            offset = rotations[id] * offset;

        bool offset_samples = is_trim_vertex
                            ? int(verts[i].Pos.z/quad_size) % 2 != 1
                            : int(verts[i].Pos.z/quad_size) % 2 != id2.y;

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
