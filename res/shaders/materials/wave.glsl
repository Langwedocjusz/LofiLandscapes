#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uWaveType;

#define WAVE_SINE 0
#define WAVE_SAW  1

uniform int uDirection;

#define DIRECTION_X 0
#define DIRECTION_Z 1

uniform int uFrequency;

uniform float uDistortion;
uniform int uOctaves;
uniform int uScale;
uniform float uRoughness;

#include "blending.glsl"
#include "common_fbm.glsl"

#define PI 3.1415926535

float Pattern(float x)
{
    switch(uWaveType)
    {
        case WAVE_SINE:
        {
            return 0.5*sin(2.0*PI*x)+0.5;   
        }
        case WAVE_SAW:
        {
            return fract(x);
        }
    }

    return 0.0;
}

float Wave(vec2 p)
{
    float fnoise = fbm(float(uScale)*p, uOctaves, uScale, uRoughness);

    float x = 0.0;

    switch(uDirection)
    {
        case DIRECTION_X:
        {
            x = p.x + uDistortion * fnoise;
            break;
        }
        case DIRECTION_Z:
        {
            x = p.y + uDistortion * fnoise;
            break;
        }
    }

    return Pattern(float(uFrequency)*x);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(heightmap);

    float prev = float(imageLoad(heightmap, texelCoord));

    float h = Wave(uv);

    h = BlendResult(prev, h);

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}