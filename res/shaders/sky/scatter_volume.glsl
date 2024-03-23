#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image3D scatterVolume;

uniform sampler2D transLUT;
uniform sampler2D multiLUT;

uniform float uHeight;

uniform vec3 uSunDir;

uniform float uFar;
uniform float uNear;

uniform vec3 uFront;

uniform vec3 uBotLeft;
uniform vec3 uBotRight;
uniform vec3 uTopLeft;
uniform vec3 uTopRight;

uniform float uDistScale;

//uniform vec3 uGroundAlbedo;

uniform int uMultiscatter;
uniform float uMultiWeight;

#include "common.glsl"

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    //Normalized 3d coordinates [0,1]
    vec3 coord = (vec3(texelCoord)+0.5)/imageSize(scatterVolume);

    //Construct ray origin
    float h = max(uHeight, 0.000001);
    vec3 org = vec3(0.0, ground_rad + h, 0.0);

    //Retrieve ray direction
    vec3 dir = mix(mix(uBotLeft, uBotRight, coord.x), mix(uTopLeft, uTopRight, coord.x), coord.y);
    dir = normalize(dir);

    //Calculate final distance

    //uNear/uFar are along z axis, so we need to project onto the ray
    //We also change units to megameters
    //And we also consider distance scale parameter
    float proj = 1.0/dot(uFront, dir);

    float t0    =            proj*0.000001*uNear;
    float t_end = uDistScale*proj*0.000001*uFar;

    float tf = t0 + coord.z*(t_end-t0);

    float dt = (tf - t0)/imageSize(scatterVolume).z;

    //Phase functions
    float cosSunAngle = dot(dir, uSunDir);        
    float mie_phase      = MiePhase(cosSunAngle);
    float rayleigh_phase = RayleighPhase(-cosSunAngle);

    //Sampling point
    vec3 p = org + tf*dir;

    //Scattering values
    float mie_s;
    vec3 rayleigh_s, extinction;

    getScatteringValues(p, rayleigh_s, mie_s, extinction);

    vec3 rayleigh_in_s = rayleigh_s * rayleigh_phase;
    float mie_in_s     = mie_s      * mie_phase;

    //Transmittance
    vec3 sample_trans = exp(-dt*extinction);
    vec3 sun_trans = getValueFromLUT(transLUT, p, uSunDir);

    //Planet (sphere) shadow
    float earth_dist = IntersectSphere(p, uSunDir, ground_rad);
    float earth_shadow = float(earth_dist < 0.0);

    //Precalculations for integration
    vec3 S = earth_shadow * sun_trans * (rayleigh_in_s + mie_in_s);

    if (uMultiscatter == 1)
    {
        //Weight is ad-hoc term temporarily added to give more control
        S += uMultiWeight * (rayleigh_s + mie_s) * getValueFromLUT(multiLUT, p, uSunDir);
    }

    vec3 IntS = S*(1.0 - sample_trans)/extinction;

    float mean_transmittance = mean(sample_trans);

    //Save
    vec4 res = vec4(IntS, mean_transmittance);

    //res = vec4(1.0, 0.0, 1.0, 1.0);

    imageStore(scatterVolume, texelCoord, res);
}