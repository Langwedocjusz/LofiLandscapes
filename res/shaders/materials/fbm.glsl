#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uOctaves;
uniform int uScale;
uniform float uRoughness;

uniform float uAmplitude;
uniform float uBias;

#include "blending.glsl"
#include "common_fbm.glsl"

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(heightmap);

    float prev = float(imageLoad(heightmap, texelCoord));

    float h = fbm(float(uScale)*uv, uOctaves, uScale, uRoughness);
    h = uAmplitude * h + uBias;

    h = BlendResult(prev, h);

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}
