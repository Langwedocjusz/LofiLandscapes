#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in uint aAuxData;

#define MAX_LEVELS 10
layout(std140, binding = 2) uniform ubo
{
    float QuadSizes[MAX_LEVELS];
};

uniform sampler2D normalmap;
uniform sampler3D aerial;

uniform vec3 uPos;
uniform mat4 uMVP;
uniform float uScaleXZ;

uniform int uFog;
uniform float uAerialDist;

out vec2 uv;
out mat3 norm_rot;
out vec3 frag_pos;
out vec4 fog_data;

#include "../common/clipmap.glsl"

mat3 rotation(vec3 N){
    vec3 T = vec3(1.0, 0.0, 0.0);
    T = normalize(T - dot(N,T)*N);
    vec3 B = cross(T, N);
    return mat3(T, N, B);
}

void main() {
    bool trim_flag = false;
    uint edge_flag = 0, lvl = 0;

    UnpackAux(aAuxData, trim_flag, edge_flag, lvl);

    float quad_size = QuadSizes[lvl];

    vec2 pos2 = GetClipmapPos(aPos.xz, uPos.xz, quad_size, trim_flag);
    vec3 pos3 = vec3(pos2.x, aPos.y, pos2.y);

    uv = (2.0/uScaleXZ) * pos2;
    uv = 0.5*uv + 0.5;

    vec3 norm = 2.0*texture(normalmap, uv).rgb - 1.0;
    norm_rot = rotation(normalize(norm));

    frag_pos = pos3;

    vec4 pos = uMVP * vec4(pos3, 1.0);

    if (uFog == 1) {
        //normalized device coordinates should be from [-1, 1], to sample fog we need [0,1]
        vec3 sampling_point = pos.xyz;
        sampling_point.xy = 0.5*sampling_point.xy/pos.w + 0.5; 
        sampling_point.z = sampling_point.z/1000;

        sampling_point.z = min(1.0, uAerialDist * sampling_point.z);
        
        fog_data = texture(aerial, sampling_point);
    }

    gl_Position = pos;
}
