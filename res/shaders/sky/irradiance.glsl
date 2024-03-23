#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform imageCube irradianceMap;

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
    
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, norm));
    up         = normalize(cross(norm, right));

    vec3 irradiance = vec3(0.0);

    float delta = 0.05;
    float samples = 0.0;

    for (float phi = 0.0; phi < 2.0*PI; phi += delta)
    {
        for (float theta = 0.0; theta < 0.5*PI; theta += delta)
        {
            vec3 sample_tangent = vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
            vec3 sample_dir = sample_tangent.x*right + sample_tangent.y*up + sample_tangent.z*norm;

            irradiance += getValFromSkyLUT(skyLUT, sample_dir, view_pos, uSunDir);
            samples += 1.0;
        }
    }

    irradiance = uSkyBrightness * PI * irradiance/samples;

    irradiance = rgb2hsv(irradiance);
    irradiance.y *= uIBLOversaturation;
    irradiance = hsv2rgb(irradiance);

    imageStore(irradianceMap, texelCoord, vec4(irradiance, 1.0));
}