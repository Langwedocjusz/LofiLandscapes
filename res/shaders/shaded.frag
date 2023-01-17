#version 450 core

#define PI 3.1415926535

#define sat(x) clamp(x, 0.0, 1.0)

in vec2 uv;
in mat3 norm_rot;
in vec3 frag_pos;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform sampler2D albedo;
uniform sampler2D normal;

uniform sampler2D skyLUT;

uniform vec3 uPos;
uniform vec3 uLightDir;

uniform int uShadow;
uniform int uMaterial;
uniform int uFixTiling;

//rgb - color, a - additional strength parameter
uniform vec4 uSunCol;
uniform vec4 uSkyCol;
uniform vec4 uRefCol;

uniform float uTilingFactor;
uniform float uNormalStrength;
uniform float uMinSkylight;

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
vec4 getTextureSample(sampler2D sampler, vec2 p, float freq, vec2 node) {
    vec3 hash = hash33(vec3(node.xy, 0.0));
    float theta = 2.0*PI*hash.x;
    float c = cos(theta), s = sin(theta);
    mat2 rot = mat2(c, s, -s, c);

    vec2 uv = rot * freq * p + hash.yz;
    return texture(sampler, uv);
}

//Interpolate result from 3 node points
vec4 TextureFixedTiling(sampler2D sampler, vec2 p, float freq) {
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
        return texture(albedo, uTilingFactor*uv);
}

vec4 getMatNormal(vec2 uv) {
    if (uFixTiling == 1)
        return TextureFixedTiling(normal, uv, uTilingFactor);
    else
        return texture(normal, uTilingFactor*uv);
}

//======================================================================
//PBR - work in progress, based on https://learnopengl.com/PBR/Lighting

float D_GGX(vec3 n, vec3 h, float a) {
    float a2     = a*a;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 F_Schlick(vec3 v, vec3 h, vec3 f0) {
    float f = pow(1.0 - sat(dot(v,h)), 5.0);
    return f0 + (1.0-f0)*f;
}

vec3 ShadePBR(vec3 view, vec3 norm, vec3 ldir,
                vec3 albedo, float roughness)
{
    const vec3 reflectance = vec3(0.04);

    //Easier to tweak parametrization
    roughness *= roughness;

    vec3 h = normalize(ldir + view);
    
    float D = D_GGX(norm, h, roughness);
    float G = GeometrySmith(norm, view, ldir, roughness);
    vec3  F = F_Schlick(view, h, reflectance); 
    
    vec3 numerator    = D * G * F;
    float denominator = 4.0 * sat(dot(norm, view)) * sat(dot(norm, ldir)) + 0.0001;
    vec3 specular     = numerator / denominator;  
    
    //Assumed metalic = 0
    vec3 kD = vec3(1.0) - F;
    
    float NoL = sat(dot(norm, ldir));

    return (kD*albedo/PI + specular) * NoL;
}

//Atm used to simulate skylighting (will try to switch to actual ibl) and reflected light
vec3 diffuseOnly(vec3 norm, vec3 ldir, vec3 albedo)
{
    float NoL = sat(dot(norm, ldir));
    return (albedo/PI) * NoL;
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

    vec3 sun_col = uSunCol.a * uSunCol.rgb;
    vec3 sky_col = uSkyCol.a * uSkyCol.rgb;
    vec3 ref_col = uRefCol.a * uRefCol.rgb;

    vec3 color = shadow * sun_col * ShadePBR(view, norm, uLightDir, albedo, roughness);
    color += amb * sky_col * diffuseOnly(norm, up, albedo);
    color += amb * ref_col * diffuseOnly(norm, -uLightDir, albedo);
    color *= mat_amb;

    //Color correction
    color = pow(color, vec3(1.0/2.2));
    //Output
    frag_col = vec4(color, 1.0);
}
