#version 450 core

struct Vert {
    vec4 Pos; //includes 4 bytes of padding
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

void main() {
    uint i = gl_GlobalInvocationID.x;

    float quad_size = verts[i].QuadSize;

    vec2 hoffset = uPos - mod(uPos, quad_size);

    vec2 pos2 = verts[i].Pos.xz;

    if (verts[i].TrimFlag == 1.0)
    {
        ivec2 id2 = ivec2(hoffset/quad_size);

        id2.x = id2.x % 2;
        id2.y = id2.y % 2;

        int id = id2.x + 2*id2.y;

        mat2 rotations[4] = mat2[4](
            mat2(1.0, 0.0, 0.0, 1.0),
            mat2(0.0, 1.0, -1.0, 0.0),
            mat2(0.0, -1.0, 1.0, 0.0),
            mat2(-1.0, 0.0, 0.0, -1.0)
        );

        pos2 = rotations[id] * pos2;
        pos2 += quad_size * vec2(1-id2.x, 1-id2.y);
    }

    vec2 uv = (2.0/uScaleXZ) * (pos2 + hoffset);
    uv = 0.5*uv + 0.5;
    
    float height = 0.5 * uScaleY * texture(heightmap, uv).r;

    verts[i].Pos.y = height;
}
