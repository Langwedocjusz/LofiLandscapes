#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aQuadSize;

layout (location = 3) in float aTrimFlag;

out vec2 uv;
out mat3 norm_rot;
out vec3 frag_pos;
out vec4 fog_data;

uniform float uScaleXZ;
uniform vec3 uPos;
uniform mat4 uMVP;

uniform sampler2D normalmap;
uniform sampler3D aerial;

uniform int uFog;

uniform float uAerialDist;

mat3 rotation(vec3 N){
    vec3 T = vec3(1.0, 0.0, 0.0);
    T = normalize(T - dot(N,T)*N);
    vec3 B = cross(T, N);
    return mat3(T, N, B);
}

void main() {
    vec2 hoffset = uPos.xz - mod(uPos.xz, aQuadSize);
    vec3 offset = vec3(hoffset.x, 0.0, hoffset.y);

    vec2 pos2 = aPos.xz;
    vec3 pos3 = aPos.xyz;

    if (aTrimFlag == 1.0)
    {
        ivec2 id2 = ivec2(hoffset/aQuadSize);

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
        pos2 += aQuadSize * vec2(1-id2.x, 1-id2.y);

        pos3.x = pos2.x;
        pos3.z = pos2.y;
    }

    uv = (2.0/uScaleXZ) * (pos2 + hoffset);
    uv = 0.5*uv + 0.5;

    vec3 norm = 2.0*texture(normalmap, uv).rgb - 1.0;
    norm_rot = rotation(normalize(norm));

    frag_pos = pos3 + offset;

    vec4 pos = uMVP * vec4(pos3 + offset, 1.0);

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
