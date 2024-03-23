#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform imageCube prefilteredMap;

uniform sampler2D skyLUT;

uniform int uResolution;
uniform vec3 uSunDir;
uniform float uSkyBrightness;
uniform float uIBLOversaturation;

#include "common.glsl"

void main() 
{
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    // 200M above the ground.
    const vec3 view_pos = vec3(0.0, ground_rad + 0.0002, 0.0);

    vec3 norm = normalize(cubeCoordToWorld(texelCoord, uResolution));

    //To-do: actual prefiltering - right now this is only raw skyview
    //that later gets default linear mips
    vec3 color = uSkyBrightness * getValFromSkyLUT(skyLUT, norm, view_pos, uSunDir);

    color = rgb2hsv(color);
    color.y *= uIBLOversaturation;
    color = hsv2rgb(color);

    imageStore(prefilteredMap, texelCoord, vec4(color, 1.0));
}