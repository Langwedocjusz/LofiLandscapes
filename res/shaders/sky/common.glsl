//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This is the common code between shaders

uniform int uResolution;

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
    
    //Extinction is a sum of absorbionts and scatterings
    extinction = rayleigh_s + rayleigh_a + mie_s + mie_a + ozone_a;
}

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