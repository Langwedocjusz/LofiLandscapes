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

uniform vec3 uCamDir;
uniform float uCamFov;
uniform float uAspectRatio; //assumed y/x

uniform float uSkyBrightness;

uniform float uHeight;

//Planet parameters
//In mega-meters by assumption
const float ground_rad = 6.360;
const float atmosphere_rad = 6.460;

const vec3 view_pos = vec3(0.0, ground_rad + uHeight, 0.0);

float safeacos(float x);
vec3 getValueFromLUT(sampler2D tex, vec3 pos, vec3 sunDir);

vec3 getValFromSkyLUT(vec3 rayDir) {
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
        vec3 right = cross(uSunDir, up);
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

vec3 getSunColor(vec3 dir)
{
    const float sunSolidAngle = 2.0*0.53*PI/180.0;
    const float minSunCosTheta = cos(sunSolidAngle);

    float cosTheta = dot(dir, uSunDir);

    //Antialiasing
    float res = smoothstep(minSunCosTheta, 1.0, cosTheta);

    //Multiplying by transmittance to get correct color
    vec3 transmittance = getValueFromLUT(transLUT, view_pos, dir);
    return res * transmittance;
}

void main() {
    float camWidthScale = 2.0*tan(uCamFov/2.0);
    float camHeightScale = camWidthScale*uAspectRatio;
    
    vec3 camRight = normalize(cross(uCamDir, vec3(0.0, 1.0, 0.0)));
    vec3 camUp = normalize(cross(camRight, uCamDir));
    
    vec2 xy = 2.0 * uv - 1.0;
    vec3 rayDir = normalize(uCamDir + camRight*xy.x*camWidthScale + camUp*xy.y*camHeightScale);
    
    vec3 color = getValFromSkyLUT(rayDir);

    color += getSunColor(rayDir);

    color *= uSkyBrightness;

    frag_col = vec4(color, 1.0);
}

//Utility
float safeacos(float x) {
    return acos(clamp(x, -1.0, 1.0));
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