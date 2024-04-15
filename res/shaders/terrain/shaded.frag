#version 450 core

in vec2 uv;
in mat3 norm_rot;
in vec3 frag_pos;
in vec4 fog_data;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;
uniform sampler2D materialmap;

uniform sampler2DArray albedo;
uniform sampler2DArray normal;

uniform samplerCube irradiance;
uniform samplerCube prefiltered;

uniform vec3 uPos;
uniform vec3 uLightDir;

uniform int uShadow;
uniform int uMaterial;
uniform int uFixTiling;
uniform int uFog;

uniform vec3 uSunCol;

uniform float uSunStr;
uniform float uSkyDiff;
uniform float uSkySpec;
uniform float uRefStr;

uniform float uTilingFactor;
uniform float uNormalStrength;

#include "../common/pbr.glsl"

#define SUM_COMPONENTS(v) (v.x + v.y + v.z + v.w)

vec4 getMaterialTexture(sampler2DArray sampler, vec2 uv_map, vec2 uv_mat) {

    vec4 weights = texture(materialmap, uv_map);
    float weight4 = 1.0 - SUM_COMPONENTS(weights);

    vec4 res = vec4(0.0);

    //Favor branching over additional texture fetches
    if (weights.x > 0.0) res += weights.x * texture(sampler, vec3(uv_mat, 0.0));
    if (weights.y > 0.0) res += weights.y * texture(sampler, vec3(uv_mat, 1.0));
    if (weights.z > 0.0) res += weights.z * texture(sampler, vec3(uv_mat, 2.0));
    if (weights.w > 0.0) res += weights.w * texture(sampler, vec3(uv_mat, 3.0));
    if (weight4 > 0.0)   res += weight4   * texture(sampler, vec3(uv_mat, 4.0));

    return res;
}

//======================================================================
//Fixing texture tiling with 3 taps by Suslik: shadertoy.com/view/tsVGRd

const vec2 hex_ratio = vec2(1.0, sqrt(3.0));

//Hex grid, explained well by Shane: shadertoy.com/view/Xljczw
//xy - center of the closest hexagon, zw - id of the closest hexagon (the same modulo coordinate change)
vec4 getHex(vec2 p) {
    vec4 hexIds = round(vec4(p, p - vec2(0.5, 1.0))/hex_ratio.xyxy);

    vec4 hexCenters = vec4(hexIds.xy * hex_ratio, 
                           (hexIds.zw + 0.5) * hex_ratio);

    vec4 offsets = p.xyxy - hexCenters;

    return dot(offsets.xy, offsets.xy) < dot(offsets.zw, offsets.zw)
        ? vec4(hexCenters.xy, hexIds.xy)
        : vec4(hexCenters.zw, hexIds.zw);
}

float getHexSDF(in vec2 p) {
    p = abs(p);
    return 0.5 - max(dot(p, 0.5*hex_ratio), p.x);
}

//xy - position of the node, z - distance to the node from p
vec3 getInterpNode(in vec2 p, in float freq, in int node_id) {
    vec2 nodeOffsets[] = vec2[](vec2(0.0, 0.0), 
                                vec2(1.0, 1.0), 
                                vec2(1.0, -1.0));
    vec2 uv = freq*p + nodeOffsets[node_id] / hex_ratio * 0.5;
    vec4 hex = getHex(uv);
    float dist = getHexSDF(uv - hex.xy) * 2.0;
    return vec3(hex.xy / freq, dist);
}

vec3 hash33(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)), 
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p)*43758.543123);
}

//Sample with random rotation and offset - generated at the node point
vec4 getTextureSample(sampler2DArray sampler, vec2 p, float freq, vec2 node) {
    vec3 hash = hash33(vec3(node.xy, 0.0));
    float theta = 2.0*PI*hash.x;
    float c = cos(theta), s = sin(theta);
    mat2 rot = mat2(c, s, -s, c);

    vec2 uv = rot * freq * p + hash.yz;

    return getMaterialTexture(sampler, p, uv);
}

//Interpolate result from 3 node points
vec4 TextureFixedTiling(sampler2DArray sampler, vec2 p, float freq) {
    vec4 res = vec4(0.0);
    
    for (int i=0; i<3; i++) {
        vec3 node = getInterpNode(p, freq, i);
        res += node.z * getTextureSample(sampler, p, freq, node.xy);
    }

    return res;
}

//======================================================================

vec4 getMatAlbedo(vec2 uv) {
    if (uFixTiling == 1)
        return TextureFixedTiling(albedo, uv, uTilingFactor);
    else 
        return getMaterialTexture(albedo, uv, uTilingFactor*uv);    
}

vec4 getMatNormal(vec2 uv) {
    if (uFixTiling == 1)
        return TextureFixedTiling(normal, uv, uTilingFactor);
    else
        return getMaterialTexture(normal, uv, uTilingFactor*uv);    
}

//======================================================================

void main() {
    const vec3 up = vec3(0.0, 1.0, 0.0);  

    vec4 res = texture(normalmap, uv);
    vec3 norm = 2.0*res.xyz - 1.0;
    float amb = res.w;

    float mat_amb = 1.0;
    float roughness = 0.7;
    vec3 albedo = vec3(1.0);

    if (uMaterial == 1) {
        vec4 mat_res = getMatNormal(uv);
        vec3 mat_norm = 2.0*mat_res.rgb-1.0;
        norm = mix(norm, norm_rot*mat_norm, uNormalStrength);
        norm = normalize(norm);
        mat_amb = mat_res.a;

        vec4 mat_alb = getMatAlbedo(uv);
        albedo = mat_alb.rgb;
        roughness = mat_alb.a;
    }

    //Band-aid solution, will have to fix normal generation
    norm = vec3(-1.0, 1.0, -1.0)*norm;

    //vec3 view = normalize(frag_pos - uPos);
    vec3 view = normalize(uPos - frag_pos);
    
    //Do pbr lighting
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, uv).r;

    vec3 sun_col = uSunStr * uSunCol;
    vec3 ref_col = uRefStr * uSunCol;

    vec3 color = shadow * sun_col * ShadePBR(view, norm, uLightDir, albedo, roughness);
    color += amb * IBL(irradiance, prefiltered, norm, view, albedo, roughness);
    color += amb * ref_col * diffuseOnly(norm, -uLightDir, albedo);
    color *= mat_amb;

    //Aerial perspective fog
    if (uFog == 1)
        color = fog_data.a * color + fog_data.rgb;

    //Color correction
    color = pow(color, vec3(1.0/2.2));

    //Output
    frag_col = vec4(color, 1.0);
}
