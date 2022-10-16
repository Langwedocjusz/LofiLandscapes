#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D tex;

uniform float uTheta;
uniform float uPhi;

void main() {
    vec4 res = texture(tex, uv);

    float sT = sin(uTheta), cT = cos(uTheta);
    float sP = sin(uPhi), cP = cos(uPhi);

    const vec3 l_dir = vec3(cP*sT, cT, sP*sT);
    vec3 norm = 2.0*res.xyz - 1.0;

    float dif = clamp(dot(l_dir, norm), 0.0, 1.0); //texture(tex, uv);
    frag_col = vec4(vec3(dif), 1.0);
    //frag_col = vec4(vec3(res.w), 1.0);
}
