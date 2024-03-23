//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This is the common code between shaders

#define PI 3.1415926535

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

//Utilities
float safeacos(float x) {
    return acos(clamp(x, -1.0, 1.0));
}

float mean(vec3 v){
    return 0.33*(v.x + v.y + v.z);
}

vec3 cubeCoordToWorld(ivec3 cubeCoord, int resolution) {
    vec2 texCoord = vec2(cubeCoord.xy) / resolution;
    texCoord = texCoord  * 2.0 - 1.0; // -1..1
    switch(cubeCoord.z) {
        case 0: return vec3(1.0, -texCoord.yx); // posx
        case 1: return vec3(-1.0, -texCoord.y, texCoord.x); //negx
        case 2: return vec3(texCoord.x, 1.0, texCoord.y); // posy
        case 3: return vec3(texCoord.x, -1.0, -texCoord.y); //negy
        case 4: return vec3(texCoord.x, -texCoord.y, 1.0); // posz
        case 5: return vec3(-texCoord.xy, -1.0); // negz
    }

    return vec3(0.0);
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

//Read SkyView LUT
vec3 getValFromSkyLUT(sampler2D skyLUT, vec3 rayDir, vec3 view_pos, vec3 sun_dir) {
    float height = length(view_pos);
    vec3 up = view_pos/height;
    
    float horizonAngle = safeacos(sqrt(height*height - ground_rad*ground_rad) / height);
    float altitudeAngle = horizonAngle - acos(dot(rayDir, up)); // Between -PI/2 and PI/2
    float azimuthAngle; // Between 0 and 2*PI
    
    if (abs(altitudeAngle) > (0.5*PI - .0001)) {
        // Looking nearly straight up or down.
        azimuthAngle = 0.0;
    } 
    
    else {
        vec3 right = cross(sun_dir, up);
        vec3 forward = cross(up, right);
        
        vec3 projectedDir = normalize(rayDir - up*dot(rayDir, up));
        float sinTheta = dot(projectedDir, right);
        float cosTheta = dot(projectedDir, forward);
        
        azimuthAngle = atan(sinTheta, cosTheta) + PI;
    }
    
    // Non-linear mapping of altitude angle. See Section 5.3 of the paper.
    float v = 0.5 + 0.5*sign(altitudeAngle)*sqrt(abs(altitudeAngle)*2.0/PI);
    
    vec2 ts = vec2(azimuthAngle / (2.0*PI), v);
    
    return texture(skyLUT, ts).rgb;
}

//Phase functions
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

//Branchless hsv <-> rgb conversions
//http://sam.hocevar.net/blog/category/glsl/

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}