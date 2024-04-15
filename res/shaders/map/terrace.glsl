#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

uniform int uNumTerrace;
uniform float uFlatness;
uniform float uStrength;

float SineStep(float x)
{
    const float pi = 3.1415926535;

    if (x<0.0) return 0.0;
    else if (x>1.0) return 1.0;
    else return 0.5*sin(pi*(x-0.5)) + 0.5;
}

float Terrace(float height)
{
    const float t = 1.0 - uFlatness;
    const float r = height - floor(height);
    const float g = t * SineStep(r/t);

    return t * floor(height) + g;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float height = float(imageLoad(heightmap, texelCoord));

    float terraced = Terrace(float(uNumTerrace)*height)/float(uNumTerrace);

    height = mix(height, terraced, uStrength);

    imageStore(heightmap, texelCoord, vec4(height));
}