#version 450 core

in vec3 frag_pos;
in vec2 world_uv;

out vec4 frag_col;

uniform vec3 uPos;
uniform vec3 uLightDir;

uniform vec3 uSunCol;

uniform float uSunStr;
uniform float uSkyDiff;
uniform float uSkySpec;
uniform float uRefStr;

uniform int uShadow;

uniform float uTilingFactor;
uniform float uMaxDepth;
uniform float uStrength;
uniform float uSway;
uniform float uTime;
uniform float uNoiseTiling;
uniform vec2 uScrollVel;
uniform float uAOMin;
uniform float uAOMax;

uniform vec3 uAlbedo;
uniform float uRoughness;
uniform float uTranslucent;

uniform sampler3D raycast_res;
uniform sampler2D noise;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform samplerCube irradiance;
uniform samplerCube prefiltered;

#include "../common/pbr.glsl"

float angleFromViewDir(vec3 dir) {
    const float fANGLES = 64.0;

    float angle = atan(-dir.z, -dir.x) + PI;
    return mod(angle + 2.0*PI/(2.0*fANGLES), 2.0*PI);
}

mat3 rotation(vec3 N){
    N = normalize(N);
    vec3 T = vec3(1.0, 0.0, 0.0);
    T = normalize(T - dot(N,T)*N);
    vec3 B = cross(T, N);
    return mat3(T, N, B);
}

void main() {
    
    vec3 view = normalize(frag_pos - uPos);

    //Retrieve noise
    vec3 uSlant = uStrength * texture(noise, uNoiseTiling*frag_pos.xz + uTime*uScrollVel).rgb;
    uSlant.y = 1.0;

    vec2 uv = frag_pos.xz + uSway*uSlant.xz;
    uv = fract(uTilingFactor * uv);

    //Consider slant
    mat3 non_ortho = mat3(vec3(1,0,0), uSlant, vec3(0,0,1));
    mat3 non_ortho_inv = inverse(non_ortho);

    mat3 transform = rotation(uSlant);
    mat3 inverted = inverse(transform);

    vec3 tmp_view = non_ortho * view;

    //Retrieve data from precomputed raycast
    float angle = angleFromViewDir(tmp_view);

    vec4 res = texture(raycast_res, vec3(uv, angle/(2.0*PI)));

    vec3 norm = res.rgb; float in_dist = res.w;

    //Discard fragments with negative depth 
    //(raycast returns negative value if nothing was hit)
    if (in_dist < 0.0)
    {
        discard;
    }

    float cosa = dot(view, normalize(view*vec3(1,0,1)));
    float dist = in_dist / cosa;

    vec3 offset = dist * view;

    offset *= non_ortho_inv;
    norm *= inverted;

    //Read world normal and AO
    vec4 world_res = texture(normalmap, world_uv);
    vec3 world_norm = world_res.rgb; float amb = world_res.a;

    //Do pbr lighting
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, world_uv).r;

    vec3 sun_col = uSunStr * uSunCol;
    vec3 ref_col = uRefStr * uSunCol;

    //vec3 albedo = uAlbedo;//vec3(0.9); 
    //float roughness = uRoughness;//0.7f;

    vec3 color = shadow * sun_col * ShadePBR(view, norm, uLightDir, uAlbedo, uRoughness);
    color += amb * IBL(irradiance, prefiltered, norm, view, uAlbedo, uRoughness);
    //color += amb * ref_col * diffuseOnly(norm, -uLightDir, uAlbedo);
    color += uTranslucent * diffuseOnly(norm, -uLightDir, uAlbedo);

    //Generate fake ao, assumes max_depth = 1.0
    float fake_depth = 1.0 - clamp(0.577*in_dist, 0.0, 1.0);
    float fake_amb = clamp((fake_depth - uAOMin)/(uAOMax - uAOMin), 0.0, 1.0);

    color *= fake_amb;

    //Color correction
    color = pow(color, vec3(1.0/2.2));

    frag_col = vec4(color, 1.0);
}
