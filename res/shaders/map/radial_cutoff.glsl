#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

uniform float uBias;
uniform float uSlope;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float height = float(imageLoad(heightmap, texelCoord));

    vec2 uv = vec2(texelCoord)/imageSize(heightmap);
    float hoffset = uBias + uSlope * dot(uv-0.5, uv-0.5);

    height = max(height - hoffset, 0.0);

    imageStore(heightmap, texelCoord, vec4(height));
}