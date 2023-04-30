#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform sampler2D heightmap;

uniform int uResolution;

uniform vec3 uCol;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);

    imageStore(albedo, texelCoord, vec4(uCol, 1.0));
}