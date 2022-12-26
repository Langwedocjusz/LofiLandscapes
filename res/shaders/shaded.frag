#version 450 core

#define PI 3.1415926535

in vec2 uv;
in mat3 norm_rot;
in vec3 frag_pos;

out vec4 frag_col;

uniform sampler2D normalmap;
uniform sampler2D shadowmap;

uniform sampler2D albedo;
uniform sampler2D normal;

uniform vec3 uPos;
//uniform float uTheta;
//uniform float uPhi;
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
    if (uFixTiling == 1) {
        return TextureFixedTiling(albedo, uv, uTilingFactor);
    }

    else
        return texture(albedo, uTilingFactor*uv);
}

vec4 getMatNormal(vec2 uv) {
    if (uFixTiling == 1) {
        return TextureFixedTiling(normal, uv, uTilingFactor);
    }

    else
        return texture(normal, uTilingFactor*uv);
}

void main() {
    //Assemble needed vectors
    vec4 res = texture(normalmap, uv);

    //float sT = sin(uTheta), cT = cos(uTheta);
    //float sP = sin(uPhi), cP = cos(uPhi);
    //const vec3 l_dir = vec3(cP*sT, cT, sP*sT);
    const vec3 l_dir = vec3(-1.0, 1.0, -1.0) * uLightDir;
    
    const vec3 up = vec3(0.0, 1.0, 0.0);

    vec3 norm = 2.0*res.xyz - 1.0;
    float amb = res.w;
    float mat_amb = 1.0;

    if (uMaterial == 1) {
        vec4 mat_res = getMatNormal(uv);
        vec3 mat_norm = 2.0*mat_res.rgb-1.0;
        norm = mix(norm, norm_rot*mat_norm, uNormalStrength);
        mat_amb = mat_res.a;
    }

    vec3 view = normalize(frag_pos - uPos);
    vec3 refl = reflect(-l_dir, norm);

    //Do basic phong lighting
    float shadow = 1.0;
    if(uShadow == 1) shadow = texture(shadowmap, uv).r;
    
    float sun_diff = clamp(dot( l_dir, norm), 0.0, 1.0);
    float sky_diff = clamp(dot( up,    norm), 0.0, 1.0);
    float ref_diff = clamp(dot(-l_dir, norm), 0.0, 1.0);

    vec3 sun_col = uSunCol.a * uSunCol.rgb;
    vec3 sky_col = uSkyCol.a * uSkyCol.rgb;
    vec3 ref_col = uRefCol.a * uRefCol.rgb;

    float shininess = 32.0;
    float spec = pow(max(dot(view, refl), 0.0), shininess);

    vec3 albedo = (uMaterial==1) ? getMatAlbedo(uv).rgb : vec3(1.0);

    vec3 color = shadow * sun_diff * sun_col * albedo;
    color += shadow * spec * sun_col * vec3(1.0);
    color += amb * max(uMinSkylight, sky_diff) * sky_col * albedo;
    color += amb * ref_diff * ref_col * albedo;
    color *= mat_amb;

    //Color correction
    color = pow(color, vec3(1.0/2.2));
    //Output
    frag_col = vec4(color, 1.0);
}
