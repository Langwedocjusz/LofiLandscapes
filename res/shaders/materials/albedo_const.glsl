#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform vec3 uCol;

#include "albedo_blending.glsl"

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec3 prev = imageLoad(albedo, texelCoord).rgb;

    vec3 res = uCol;

    res = BlendResult(prev, res);

    imageStore(albedo, texelCoord, vec4(res, 1.0));
}