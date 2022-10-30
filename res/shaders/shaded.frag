#version 450 core

in vec2 uv;
in mat3 norm_rot;
in vec3 frag_pos;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform sampler2D albedo;
uniform sampler2D normal;

uniform vec3 uPos;
uniform float uTheta;
uniform float uPhi;

uniform int uShadow;
uniform int uMaterial;

uniform vec3 sun_col;
uniform vec3 sky_col;
uniform vec3 ref_col;

uniform float uTilingFactor;
uniform float uNormalStrength;

void main() {
    //Assemble needed vectors
    vec4 res = texture(normalmap, uv);

    float sT = sin(uTheta), cT = cos(uTheta);
    float sP = sin(uPhi), cP = cos(uPhi);

    const vec3 l_dir = vec3(cP*sT, cT, sP*sT);
    const vec3 up = vec3(0.0, 1.0, 0.0);

    vec3 norm = 2.0*res.xyz - 1.0;

    if (uMaterial == 1) {
        vec3 mat_norm = 2.0*texture(normal, uTilingFactor*uv).rgb - 1.0;
        norm = mix(norm, norm_rot*mat_norm, uNormalStrength);
    }

    vec3 view = normalize(frag_pos - uPos);
    vec3 refl = reflect(-l_dir, norm);

    //Do basic phong lighting
    float amb = res.w;
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, uv).r;
    
    float sun_diff = clamp(dot( l_dir, norm), 0.0, 1.0);
    float sky_diff = clamp(dot( up,    norm), 0.0, 1.0);
    float ref_diff = clamp(dot(-l_dir, norm), 0.0, 1.0);

    float shininess = 32.0;
    float spec = pow(max(dot(view, refl), 0.0), shininess);

    vec3 color = shadow * (sun_diff+spec) * sun_col
               + amb * (sky_col*sky_diff + ref_col*ref_diff);
    
    if (uMaterial == 1) {
        vec3 alb = texture(albedo, uTilingFactor*uv).rgb;
        color *= alb;
    }

    //Color correction
    color = pow(color, vec3(1.0/2.2));
    //Output
    frag_col = vec4(color, 1.0);
}
