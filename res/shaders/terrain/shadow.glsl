#version 450 core

#define PI 3.1415926535

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r8, binding = 0) uniform image2D shadowmap;

uniform int uResolution;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
//uniform float uTheta;
//uniform float uPhi;
uniform vec3 uSunDir;

uniform int uSteps;
uniform float uMinT;
uniform float uMaxT;
uniform float uBias;

float getHeight(vec2 p, float lvl) {
    return textureLod(heightmap, p, lvl).r;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    //Generate ray:
    float dt = uMaxT/float(uSteps);
    float voffset = dt;

    vec3 org = vec3(uv.x, getHeight(uv, 0.0) + voffset, uv.y);

    //vec3 dir = normalize(vec3(uScaleY *cos(uPhi)*sin(uTheta),
    //                          uScaleXZ*cos(uTheta),
    //                          uScaleY *sin(uPhi)*sin(uTheta)));
    //dir *= vec3(-1.0, 1.0, -1.0);

    vec3 dir = normalize(vec3(uScaleY * uSunDir.x,
                              uScaleXZ* uSunDir.y,
                              uScaleY * uSunDir.z));

    vec2 perp = mat2(0.0, 1.0, -1.0, 0.0) * normalize(dir.xz);

    //Raymarch 
    float t = 0.0;
    float shadow = 1.0;

    for (int i=0; i<uSteps; i++) {
        vec3 p = org + t*dir;
        float h = p.y - getHeight(p.xz, uBias*t);
        float blur = 1.0/t;
        shadow = min(shadow, 1.0 - saturate(-blur*h));
        t += dt;
    }

    imageStore(shadowmap, texelCoord, vec4(shadow, 0.0, 0.0, 1.0));
}
