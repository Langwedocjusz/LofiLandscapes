#version 450 core

layout (location = 0) in vec4 aPos;

out vec2 uv;
out mat3 norm_rot;
out vec3 frag_pos;

uniform float uL;
uniform vec3 uPos;
uniform mat4 uMVP;

uniform sampler2D normalmap;

mat3 rotation(vec3 x, vec3 y) {
    mat3 I = mat3(1.0);
    vec3 v = cross(x,y);
    mat3 V = mat3(0.0,-v.z, v.y,
                  v.z, 0.0,-v.x,
                 -v.y, v.x, 0.0);
    float c = dot(x,y);
    float C = 1.0/(1.0+c);
    
    return I + V + C*V*V;
}

void main() {
    vec2 hoffset = uPos.xz - mod(uPos.xz, aPos.w);

    uv = (2.0/uL) * (aPos.xz + hoffset);
    uv = 0.5*uv + 0.5;

    vec3 norm = 2.0*texture(normalmap, uv).rgb - 1.0;
    norm_rot = rotation(vec3(0.0, 1.0, 0.0), norm);

    frag_pos = aPos.xyz;

    gl_Position = uMVP * vec4(aPos.xyz + vec3(hoffset.x, 0.0, hoffset.y), 1.0);
}
