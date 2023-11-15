#version 450 core

struct Vert {
    vec4 pos;
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

uniform float uModScale;

void main() {
    uint i = gl_GlobalInvocationID.x;

    vec2 hoffset = uPos - mod(uPos, verts[i].pos.w);
    //vec2 hoffset = uPos - mod(uPos, uModScale);

    vec2 uv = (2.0/uScaleXZ) * (verts[i].pos.xz + hoffset);
    uv = 0.5*uv + 0.5;
    
    float height = 0.5 * uScaleY * texture(heightmap, uv).r;

    verts[i].pos.y = height;
}
