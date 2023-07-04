#version 450 core

layout (location = 0) in vec4 aPos;

out vec2 uv;
out mat3 norm_rot;
out vec3 frag_pos;
out vec4 fog_data;

uniform float uL;
uniform vec3 uPos;
uniform mat4 uMVP;

uniform sampler2D normalmap;
uniform sampler3D aerial;

uniform int uFog;

mat3 rotation(vec3 N){
    vec3 T = vec3(1.0, 0.0, 0.0);
    T = normalize(T - dot(N,T)*N);
    vec3 B = cross(T, N);
    return mat3(T, N, B);
}

void main() {
    vec2 hoffset = uPos.xz - mod(uPos.xz, aPos.w);

    uv = (2.0/uL) * (aPos.xz + hoffset);
    uv = 0.5*uv + 0.5;

    vec3 norm = 2.0*texture(normalmap, uv).rgb - 1.0;
    norm_rot = rotation(normalize(norm));

    frag_pos = aPos.xyz + vec3(hoffset.x, 0.0, hoffset.y);;

    vec4 pos = uMVP * vec4(aPos.xyz + vec3(hoffset.x, 0.0, hoffset.y), 1.0);

    if (uFog == 1) {
        //normalized device coordinates should be from [-1, 1], to sample fog we need [0,1]
        vec3 sampling_point = pos.xyz;
        sampling_point.xy = 0.5*sampling_point.xy/pos.w + 0.5; 
        sampling_point.z = sampling_point.z/1000;
        
        fog_data = texture(aerial, sampling_point);
    }

    gl_Position = pos;
}
