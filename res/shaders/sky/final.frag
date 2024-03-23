//Physically based sky rendering following the paper 
//"Production Ready Atmosphere Rendering" by Sebastien Hillaire.
//Also heavilly based on Shadertoy implementation by AndrewHelmer:
//https://www.shadertoy.com/view/slSXRW

//This shader implements final sky rendering

#version 450 core

#define PI 3.1415926535

//From [0,1]
in vec2 uv;

out vec4 frag_col;

uniform sampler2D transLUT;
uniform sampler2D skyLUT;

uniform vec3 uSunDir;

uniform vec3 uBotLeft;
uniform vec3 uBotRight;
uniform vec3 uTopLeft;
uniform vec3 uTopRight;

uniform float uSkyBrightness;

uniform float uHeight;

#include "common.glsl"

vec3 getSunColor(vec3 dir)
{
    const float sunSolidAngle = 2.0*0.53*PI/180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    vec3 view_pos = vec3(0.0, ground_rad + uHeight, 0.0);
    
    float cosTheta = dot(dir, uSunDir);

    //Antialiasing
    float res = smoothstep(minSunCosTheta, 1.0, cosTheta);

    //Multiplying by transmittance to get correct color
    vec3 transmittance = getValueFromLUT(transLUT, view_pos, dir);
    return res * transmittance;
}

void main() {
    vec3 rayDir = mix(mix(uBotLeft, uBotRight, uv.x), mix(uTopLeft, uTopRight, uv.x), uv.y);
    rayDir = normalize(rayDir);
    
    vec3 view_pos = vec3(0.0, ground_rad + uHeight, 0.0);

    vec3 color = getValFromSkyLUT(skyLUT, rayDir, view_pos, uSunDir);

    color += getSunColor(rayDir);

    color *= uSkyBrightness;

    frag_col = vec4(color, 1.0);
}
