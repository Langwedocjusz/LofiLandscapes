#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform imageCube prefilteredMap;

uniform sampler2D skyLUT;

uniform int uResolution;
uniform vec3 uSunDir;
uniform float uSkyBrightness;

//Planet parameters
//In mega-meters by assumption
const float ground_rad = 6.360;

// 200M above the ground.
const vec3 view_pos = vec3(0.0, ground_rad + 0.0002, 0.0);

//Utility
float safeacos(const float x) {
    return acos(clamp(x, -1.0, 1.0));
}

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

vec3 cubeCoordToWorld(ivec3 cubeCoord) {
    vec2 texCoord = vec2(cubeCoord.xy) / uResolution;
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

void main() 
{
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    vec3 norm = normalize(cubeCoordToWorld(texelCoord));

    //To-do: actual prefiltering - right now this is only raw skyview
    //that later gets default linear mips
    vec3 color = uSkyBrightness * getValFromSkyLUT(norm);

    imageStore(prefilteredMap, texelCoord, vec4(color, 1.0));
}