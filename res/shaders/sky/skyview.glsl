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

uniform int uResolution;

uniform vec3 uSunDir;

//Planet parameters
//In mega-meters by assumption
const float ground_rad = 6.360;
const float atmosphere_rad = 6.460;

//Atmosphere parameters
//These are per megameter
const vec3 base_rayleigh_s = vec3(5.802, 13.558, 33.1);
const float base_rayleigh_a = 0.0;

const float base_mie_s = 3.996;
const float base_mie_a = 4.4;

const vec3 base_ozone_a = vec3(0.650, 1.881, .085);

// 200M above the ground.
const vec3 view_pos = vec3(0.0, ground_rad + 0.0002, 0.0);

//Common code between sky shaders
//To-do: add #include support to shader class
float safeacos(float x);
float IntersectSphere(vec3 ro, vec3 rd, float rad);
void getScatteringValues(vec3 pos, inout vec3 rayleigh_s, inout float mie_s, inout vec3 extinction);

//Read Transmittance/Multiscatter LUT
vec3 getValueFromLUT(sampler2D tex, vec3 pos, vec3 sunDir) {
    float height = length(pos);
    vec3 up = pos / height;
	float sunCosZenithAngle = dot(sunDir, up);

    //This is from [0,1]
    vec2 uv;
    uv.x = clamp(0.5 + 0.5*sunCosZenithAngle, 0.0, 1.0);
    uv.y = clamp((height - ground_rad)/(atmosphere_rad - ground_rad), 0.0, 1.0);
    
    return texture(tex, uv).rgb;
}

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
    
	float mie_phase = MiePhase(cosTheta);
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
        vec3 psiMS = getValueFromLUT(multiLUT, p, sun_dir);
        
        vec3 rayleighInScattering = rayleigh_s * (rayleigh_phase * sun_trans + psiMS);
        vec3 mieInScattering = mie_s * (mie_phase * sun_trans + psiMS);
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
    vec2 uv = vec2(texelCoord)/float(uResolution);

    //From [-pi, pi]
    float azimuthAngle = PI*2.0*(uv.x - 0.5);

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
    float cosAzi = cos(azimuthAngle), sinAzi = sin(azimuthAngle);
    
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

//Utility
float safeacos(float x) {
    return acos(clamp(x, -1.0, 1.0));
}

//Ray-sphere intersection from
//https://gamedev.stackexchange.com/questions/96459/fast-ray-sphere-collision-code.
float IntersectSphere(vec3 ro, vec3 rd, float rad) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - rad*rad;
    if (c > 0.0 && b > 0.0) return -1.0;
    float discr = b*b - c;
    if (discr < 0.0) return -1.0;
    // Special case: inside sphere, use far discriminant
    if (discr > b*b) return (-b + sqrt(discr));
    return -b - sqrt(discr);
}

void getScatteringValues(vec3 pos, inout vec3 rayleigh_s, inout float mie_s, inout vec3 extinction) {
    //Height in km
    float altitude = (length(pos)-ground_rad)*1000.0;
    
    //Density(height) distributions
    // Note: Paper gets these switched up.
    float rayleigh_dens = exp(-altitude/8.0);
    float mie_dens = exp(-altitude/1.2);
    
    rayleigh_s = base_rayleigh_s * rayleigh_dens;
    float rayleigh_a = base_rayleigh_a * rayleigh_dens;
    
    mie_s = base_mie_s * mie_dens;
    float mie_a = base_mie_a * mie_dens;
    
    //Ozone - uniform, triangle distribution
    vec3 ozone_a = base_ozone_a * max(0.0, 1.0 - abs(altitude-25.0)/15.0);
    
    //Extinction is a sum of absorbtions and scatterings
    extinction = rayleigh_s + rayleigh_a + mie_s + mie_a + ozone_a;
}