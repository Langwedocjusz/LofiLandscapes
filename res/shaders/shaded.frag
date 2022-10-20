#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform float uTheta;
uniform float uPhi;
uniform int uShadow;

void main() {
    vec4 res = texture(normalmap, uv);
    
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, uv).r;

    float sT = sin(uTheta), cT = cos(uTheta);
    float sP = sin(uPhi), cP = cos(uPhi);

    const vec3 l_dir = vec3(cP*sT, cT, sP*sT);
    vec3 norm = 2.0*res.xyz - 1.0;

    float amb = 0.2*res.w;
    float dif = 0.8*clamp(dot(l_dir, norm), 0.0, 1.0); //texture(tex, uv);

    frag_col = vec4(vec3(amb + shadow*dif), 1.0);
    //frag_col = vec4(vec3(shadow), 1.0);
}
