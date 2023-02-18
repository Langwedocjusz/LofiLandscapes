//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This shader implements Transmittance LUT

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image2D transLUT;

#include common.glsl

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
    vec2 uv = (vec2(texelCoord)+0.5)/float(uResolution);

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