//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This shader implements SkyView LUT

#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image2D skyLUT;

uniform sampler2D transLUT;
uniform sampler2D multiLUT;

uniform vec3 uSunDir;

uniform float uHeight;

#include common.glsl

const vec3 view_pos = vec3(0.0, ground_rad + uHeight, 0.0);

//Scattering phase functions
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

vec3 RaymarchScattering(vec3 pos, vec3 ray_dir, vec3 sun_dir, float t_max) {
    const int num_steps = 32;

    float cosTheta = dot(ray_dir, sun_dir);
    
	float mie_phase      = MiePhase(cosTheta);
	float rayleigh_phase = RayleighPhase(-cosTheta);
    
    vec3 lum = vec3(0.0);
    vec3 trans = vec3(1.0);

    float dt = t_max/float(num_steps);
    float t = 0.3*dt;

    for (int i = 0; i < num_steps; i++) {
        vec3 p = pos + t*ray_dir;
        
        vec3 rayleigh_s, extinction;
        float mie_s;

        getScatteringValues(p, rayleigh_s, mie_s, extinction);
        
        vec3 sample_trans = exp(-dt*extinction);

        vec3 sun_trans = getValueFromLUT(transLUT, p, sun_dir);
        vec3 psiMS     = getValueFromLUT(multiLUT, p, sun_dir);
        
        vec3 rayleighInScattering = rayleigh_s * (rayleigh_phase * sun_trans + psiMS);
        vec3 mieInScattering      = mie_s      * (mie_phase      * sun_trans + psiMS);
        vec3 inScattering = (rayleighInScattering + mieInScattering);

        // Integrated scattering within path segment.
        vec3 scattering_integral = (inScattering - inScattering * sample_trans) / extinction;

        lum += scattering_integral*trans;
        trans *= sample_trans;

        t += dt;
    }

    return lum;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    //Normalized coordinates (square texture)
    vec2 uv = (vec2(texelCoord)+0.5)/float(uResolution);

    //From [-pi, pi]
    float azimuthAngle = 2.0*PI*(uv.x - 0.5);

    //Non-linear altitude mapping (section 5.3 in the paper)
    float adjV;

    if (uv.y < 0.5) {
        float coord = 1.0 - 2.0*uv.y;
        adjV = - coord*coord;
    }

    else {
        float coord = 2.0*uv.y - 1.0;
        adjV = coord*coord;
    }

    float height = length(view_pos);
    vec3 up = view_pos/height;

    float horizonAngle = safeacos(sqrt(height*height - ground_rad*ground_rad)/height) - 0.5*PI;
    float altitudeAngle = 0.5*PI*adjV - horizonAngle; 

    float cosAlt = cos(altitudeAngle), sinAlt = sin(altitudeAngle);
    float cosAzi = cos(azimuthAngle) , sinAzi = sin(azimuthAngle);
    
    vec3 ray_dir = vec3(cosAlt*sinAzi, sinAlt, -cosAlt*cosAzi);

    float sunAlt = 0.5*PI - acos(dot(uSunDir, up));
    vec3 sun_dir = vec3(0.0, sin(sunAlt), -cos(sunAlt));

    float atm_dist = IntersectSphere(view_pos, ray_dir, atmosphere_rad);
    float gnd_dist = IntersectSphere(view_pos, ray_dir, ground_rad);

    float t_max = (gnd_dist < 0.0) ? atm_dist : gnd_dist;

    vec3 lum = RaymarchScattering(view_pos, ray_dir, sun_dir, t_max);

    vec4 res = vec4(lum, 1.0);

    imageStore(skyLUT, texelCoord, res);
}