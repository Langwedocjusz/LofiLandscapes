//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.

//This shader generates the aerial perspective lut
//Work in progress

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image3D aerialLUT;

uniform sampler2D transLUT;
uniform sampler2D multiLUT;

uniform sampler2D shadowmap;

#include "common.glsl"

uniform float uHeight;

uniform vec3 uSunDir;

uniform float uFar;
uniform float uNear;

uniform vec3 uFront;

uniform vec3 uBotLeft;
uniform vec3 uBotRight;
uniform vec3 uTopLeft;
uniform vec3 uTopRight;

uniform float uBrightness;
uniform float uDistScale;

//uniform vec3 uGroundAlbedo;

uniform int uMultiscatter;
uniform float uMultiWeight;

uniform vec3 uPos;
uniform int uShadows;

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    //Normalized 3d coordinates [0,1]
    vec3 coord = (vec3(texelCoord)+0.5)/imageSize(aerialLUT);

    //Construct ray origin
    float h = max(uHeight, 0.000001);
    vec3 org = vec3(0.0, ground_rad + h, 0.0);

    //Retrieve ray direction
    vec3 dir = mix(mix(uBotLeft, uBotRight, coord.x), mix(uTopLeft, uTopRight, coord.x), coord.y);
    dir = normalize(dir);

    //Calculate initial and final distances

    //uNear/uFar are along z axis, so we need to project onto the ray
    //We also change units to megameters
    //And we also consider distance scale parameter
    float proj = 1.0/dot(uFront, dir);

    float t0    =            proj*0.000001*uNear;
    float t_end = uDistScale*proj*0.000001*uFar;

    float tf = t0 + coord.z*(t_end-t0);

    //Calculate step size
    const int samples_per_voxel = 5;
    
    int num_steps = samples_per_voxel * int(gl_GlobalInvocationID.z) //uint->int (small so should be fine)
                  + (samples_per_voxel + (samples_per_voxel % 2))/2;
    
    float dt = (tf - t0)/float(num_steps);

    //Consider intersections with the planet/atmosphere
    float atm_dist = IntersectSphere(org, dir, atmosphere_rad);
    float gnd_dist = IntersectSphere(org, dir, ground_rad);

    //If we didn't hit the atmosphere we may as well exit early
    if (atm_dist < 0.0)
    {
        vec4 res = vec4(vec3(0.0), 1.0);
        imageStore(aerialLUT, texelCoord, res);
        return;
    }

    float t_cutoff = atm_dist;

    //Select minimum for the cutoff dist
    if (gnd_dist > 0.0)
    {
        t_cutoff = min(t_cutoff, gnd_dist);
    }

    //Phase functions
    float cosSunAngle = dot(dir, uSunDir);        
    float mie_phase      = MiePhase(cosSunAngle);
    float rayleigh_phase = RayleighPhase(-cosSunAngle);

    //Raymarching
    float transmittance = 1.0;
    vec3 in_scatter = vec3(0.0);

    //Start at the beginning of the frustum
    float t = t0;

    for (int i=0; i<= num_steps; i++)
    {
        vec3 p = org + t*dir;

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

        //Integration
        vec3 S = earth_shadow * sun_trans * (rayleigh_in_s + mie_in_s);

        if (uMultiscatter == 1)
        {
            //Weight is ad-hoc term temporarily added to give more control
            S += uMultiWeight * (rayleigh_s + mie_s) * getValueFromLUT(multiLUT, p, uSunDir);
        }

        vec3 IntS = S*(1.0 - sample_trans)/extinction;

        in_scatter += transmittance * IntS;
        transmittance *= mean(sample_trans);

        t += dt;

        if (t >= t_cutoff) break;
    }

    //Account for the light that bounced off the planet
    //if (gnd_dist > 0.0)
	//{
	//	vec3 p = org + gnd_dist * dir;
    //
	//	vec3 sun_trans = getValueFromLUT(transLUT, p, sun_dir);
    //
    //    //Normal to the planet is just normalized p
    //    vec3 norm = normalize(p);
    //
	//	float NdotL = clamp(dot(norm, sun_dir), 0.0, 1.0);
    //
	//	//in_scatter += sun_trans * transmittance * NdotL * uGroundAlbedo / PI;
    //}

    //Save
    vec4 res = vec4(uBrightness*in_scatter, transmittance);

    imageStore(aerialLUT, texelCoord, res);
}