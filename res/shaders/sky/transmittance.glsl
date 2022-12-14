//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This shader implements Transmittance LUT

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image2D transLUT;

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

//Common code between sky shaders
//To-do: add #include support to shader class
float safeacos(float x);
float IntersectSphere(vec3 ro, vec3 rd, float rad);
void getScatteringValues(vec3 pos, inout vec3 rayleigh_s, inout float mie_s, inout vec3 extinction);

vec3 Transmittance(vec3 pos, vec3 dir) {
    const int steps = 40;

    //Ray is hitting the Earth - no transmittance
    if (IntersectSphere(pos, dir, ground_rad) > 0.0)
        return vec3(0.0);

    //Distance to edge of the atmosphere
    float atm_dist = IntersectSphere(pos, dir, atmosphere_rad);

    vec3 res = vec3(1.0);
    
    //Integrate transmittance
    float dt = atm_dist/float(steps);
    float t = 0.3*dt; //starting offset

    for (int i = 0; i<steps; i++) {
        vec3 p = pos + t*dir;

        vec3 rayleigh_s, extinction;
        float mie_s;
        
        getScatteringValues(p, rayleigh_s, mie_s, extinction);

        //Beer-Lambert law
        res *= exp(-dt*extinction);

        t += dt;
    }
    
    return res;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    //Normalized coordinates (square texture)
    vec2 uv = vec2(texelCoord)/float(uResolution);

    //Convert to (sun zenith angle, height above ground)
    float sunAngleCos = 2.0 * uv.x - 1.0;
    float sunAngle = safeacos(sunAngleCos);

    float height = mix(ground_rad, atmosphere_rad, uv.y);

    //Recover 3d position and sun direction
    vec3 pos = vec3(0.0, height, 0.0);
    vec3 dir = normalize(vec3(
        0.0, sunAngleCos, -sin(sunAngle)
    ));

    //Calculate rgb transmittance
    vec4 res = vec4(Transmittance(pos, dir), 1.0);

    imageStore(transLUT, texelCoord, res);
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
    
    //Extinction is a sum of absorbionts and scatterings
    extinction = rayleigh_s + rayleigh_a + mie_s + mie_a + ozone_a;
}