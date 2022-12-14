//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This shader implements Multiscattering LUT

#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image2D multiLUT;

uniform sampler2D transLUT;

uniform int uResolution;

//Planet parameters
//In mega-meters by assumption
const float ground_rad = 6.360;
const float atmosphere_rad = 6.460;

uniform vec3 uGroundAlbedo;

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

//3d Direction from spherical coords
vec3 SphericalDir(float theta, float phi) {
    float cT = cos(theta), sT = sin(theta);
    float cP = cos(phi)  , sP = sin(phi);

    return vec3(sP*sT, cP, sP*cT);
}

vec3 MultiScatter(vec3 pos, vec3 sun_dir) {
    const int sqrt_samples = 8, mult_steps = 20;
    float inv_samples = 1.0/float(sqrt_samples*sqrt_samples);

    vec3 lum_tot = vec3(0.0), fms = vec3(0.0);

    //Secondary scattering double integral over directions
    for (int i=0; i<sqrt_samples; i++) {

        for (int j=0; j<sqrt_samples; j++) {
            float theta = PI * (float(i)+0.5)/float(sqrt_samples);
            float phi = safeacos(1.0 - 2.0*(float(j)+0.5)/float(sqrt_samples));
        
            vec3 ray_dir = SphericalDir(theta, phi);

            float atm_dist = IntersectSphere(pos, ray_dir, atmosphere_rad);
            float gnd_dist = IntersectSphere(pos, ray_dir, ground_rad);
        
            float t_max = atm_dist;
            if (gnd_dist > 0.0) {
                t_max = gnd_dist;
            }

            float cosSunAngle = dot(ray_dir, sun_dir);
            
            float mie_phase      = MiePhase(cosSunAngle);
            float rayleigh_phase = RayleighPhase(-cosSunAngle);

            vec3 lum = vec3(0.0), lum_fac = vec3(0.0);
            vec3 trans = vec3(1.0);

            //Inner integration along fixed direction
            float dt = t_max/float(mult_steps);
            float t = 0.3*dt; //initial offset

            for (int k=0; k<mult_steps; k++) {
                vec3 p = pos + t*ray_dir;

                vec3 rayleigh_s, extinction;
                float mie_s;

                getScatteringValues(p, rayleigh_s, mie_s, extinction);

                vec3 sample_trans = exp(-dt*extinction);

                vec3 scatter_no_phase = rayleigh_s + mie_s;
                vec3 scatter_F = scatter_no_phase*(1.0-sample_trans)/extinction;

                lum_fac += trans * scatter_F;

                vec3 sun_trans = getValueFromLUT(transLUT, p, sun_dir);

                vec3 rayleigh_in_s = rayleigh_s * rayleigh_phase;
                float mie_in_s     = mie_s      * mie_phase;

                vec3 in_scatter = (rayleigh_in_s + mie_in_s)*sun_trans;

                vec3 scatter_integral = in_scatter*(1.0-sample_trans)/extinction;

                lum += scatter_integral*trans;
                trans *= sample_trans;

                t += dt;
            }

            if (gnd_dist > 1.0) {
                vec3 hit_point = pos + gnd_dist*ray_dir;
                if (dot(pos, sun_dir) > 0.0) {
                    hit_point = normalize(hit_point)*ground_rad;
                    lum += trans*uGroundAlbedo*getValueFromLUT(transLUT, hit_point, sun_dir);
                }
            }

            fms += lum_fac * inv_samples;
            lum_tot += lum * inv_samples;
        }
    }

    //Equation 10 from the paper (geometric series)
    vec3 psi = lum_tot/(1.0 - fms);
    return psi;
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

    //Calculate multi scattering
    vec4 res = vec4(MultiScatter(pos, dir), 1.0);

    imageStore(multiLUT, texelCoord, res);
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