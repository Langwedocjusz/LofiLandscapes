//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.

//This shader generates the aerial perspective lut
//Work in progress

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image3D aerialLUT;

uniform sampler2D transLUT;

#include "common.glsl"

uniform float uHeight;

uniform vec3 uSunDir;

uniform float uFar;
uniform float uFov;
uniform float uAspect;//y/x

uniform vec3 uFront;
uniform vec3 uRight;
uniform vec3 uTop;

uniform float uBrightness;
uniform float uDistScale;

float MiePhase(float cosTheta) {
    const float g = 0.8;
    const float scale = 3.0/(8.0*PI);
    
    float num = (1.0-g*g)*(1.0+cosTheta*cosTheta);
    float denom = (2.0+g*g)*pow((1.0 + g*g - 2.0*g*cosTheta), 1.5);
    
    return scale*num/denom;
}

float RayleighPhase(float cosTheta) {
    const float k = 3.0/(16.0*PI);
    return k*(1.0+cosTheta*cosTheta);
}

float mean(vec3 v){
    return 0.33*(v.x + v.y + v.z);
}

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    //Normalized 3d coordinates [0,1]
    vec3 coord = (vec3(texelCoord)+0.5)/float(uResolution); //assumes cubic shape

    //Plane coordinates [-1;1]
    vec2 uv = 2.0 * coord.xy - 1.0;

    //Ray data
    const int max_steps = 128;
    int num_steps = int(float(max_steps) * coord.z);

    float h = max(uHeight, 0.000001);

    vec3 org = vec3(0.0, ground_rad + h, 0.0);
    vec3 dir = (uFront + tan(uFov)*(uv.x*uRight + uAspect*uv.y*uTop)); //not normalized on purpose
    vec3 norm_dir = normalize(dir);

    float conv_fac = dot(dir, uFront)/length(dir);

    //Should be in megameters
    float t_max = (uDistScale * 0.000001 * uFar) * coord.z;
    float dt = t_max / float(num_steps);
    float dt_phys = dt/conv_fac;

    //Distance cutoff due to planet/atmosphere geometry
    float atm_dist = IntersectSphere(org, norm_dir, atmosphere_rad);
    float gnd_dist = IntersectSphere(org, norm_dir, ground_rad);

    float max_t = conv_fac * atm_dist;
    if (gnd_dist > 0.0) max_t = min(max_t, conv_fac * gnd_dist);

    //Phase functions
    vec3 sun_dir = vec3(-1,1,-1) * uSunDir;
    float cosSunAngle = dot(norm_dir, sun_dir);        
    float mie_phase      = MiePhase(-cosSunAngle);
    float rayleigh_phase = RayleighPhase(cosSunAngle);

    //Raymarching
    float t = 0.0; //distance along z axis

    float transmittance = 1.0;
    vec3 in_scatter = vec3(0.0);

    for (int i=0; i<num_steps; i++) {
        vec3 p = org + t*dir;

        //Scattering values
        float mie_s;
        vec3 rayleigh_s, extinction;

        getScatteringValues(p, rayleigh_s, mie_s, extinction);

        vec3 rayleigh_in_s = rayleigh_s * rayleigh_phase;
        float mie_in_s     = mie_s      * mie_phase;

        //Transmittance
        vec3 sample_trans = exp(-dt_phys*extinction);
        vec3 sun_trans = getValueFromLUT(transLUT, p, sun_dir);

        //Earth shadow
        float earth_dist = IntersectSphere(p, sun_dir, ground_rad);
        float earth_shadow = float(earth_dist < 0.0);

        //Integration
        vec3 S = earth_shadow * sun_trans * (rayleigh_in_s + mie_in_s);
        vec3 IntS = S*(1.0 - sample_trans)/extinction;

        in_scatter += transmittance * IntS;
        transmittance *= mean(sample_trans);

        t += dt;

        if (t >= max_t) break;
    }

    //Save
    vec4 res = vec4(uBrightness*in_scatter, transmittance);

    imageStore(aerialLUT, texelCoord, res);
}