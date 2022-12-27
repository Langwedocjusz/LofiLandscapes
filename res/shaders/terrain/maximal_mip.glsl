#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

//Lower mip means higher res
layout(r32f, binding = 1) uniform image2D lower_mip; 

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float h =  imageLoad(lower_mip, 2*texelCoord + ivec2(0,0)).r;
    h = max(h, imageLoad(lower_mip, 2*texelCoord + ivec2(1,0)).r);
    h = max(h, imageLoad(lower_mip, 2*texelCoord + ivec2(0,1)).r);
    h = max(h, imageLoad(lower_mip, 2*texelCoord + ivec2(1,1)).r);

    imageStore(heightmap, texelCoord, vec4(h));
}