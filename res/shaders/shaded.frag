#version 450 core

in vec2 uv;
in vec3 frag_pos;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform vec3 uPos;
uniform float uTheta;
uniform float uPhi;
uniform int uShadow;

uniform vec3 sun_col;
uniform vec3 sky_col;
uniform vec3 ref_col;

uniform float shininess;

void main() {
    //Assemble needed vectors
    vec4 res = texture(normalmap, uv);

    float sT = sin(uTheta), cT = cos(uTheta);
    float sP = sin(uPhi), cP = cos(uPhi);

    const vec3 l_dir = vec3(cP*sT, cT, sP*sT);
    const vec3 up = vec3(0.0, 1.0, 0.0);

    vec3 norm = 2.0*res.xyz - 1.0;
    vec3 view = normalize(frag_pos - uPos);
    vec3 refl = reflect(-l_dir, norm);

    //Do basic phong lighting
    float amb = res.w;
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, uv).r;
    
    float sun_diff = clamp(dot( l_dir, norm), 0.0, 1.0);
    float sky_diff = clamp(dot( up,    norm), 0.0, 1.0);
    float ref_diff = clamp(dot(-l_dir, norm), 0.0, 1.0);

    float spec = pow(max(dot(view, refl), 0.0), shininess);

    vec3 color = shadow * (sun_diff+spec) * sun_col
               + amb * (sky_col*sky_diff + ref_col*ref_diff);
    
    //Color correction
    color = pow(color, vec3(1.0/2.2));
    //Output
    frag_col = vec4(color, 1.0);
}
