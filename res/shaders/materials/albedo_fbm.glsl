#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform int uOctaves;
uniform int uScale;
uniform float uRoughness;

uniform float uEdge1;
uniform float uEdge2;

uniform vec3 uCol1;
uniform vec3 uCol2;

#include "common_fbm.glsl"
#include "albedo_blending.glsl"

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(albedo);
    
    vec3 prev = imageLoad(albedo, texelCoord).rgb;

    float h = fbm(float(uScale)*uv, uOctaves, uScale, uRoughness);

    float fac = saturate((h-uEdge1)/(uEdge2-uEdge1));

    vec3 res = mix(uCol1, uCol2, fac);

    res = BlendResult(prev, res);

    imageStore(albedo, texelCoord, vec4(res, 1.0));
}
