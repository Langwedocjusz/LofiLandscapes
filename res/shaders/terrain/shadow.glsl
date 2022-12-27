#version 450 core

//#define FANCY

#define PI 3.1415926535

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r8, binding = 0) uniform image2D shadowmap;

uniform int uResolution;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
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

    float texel_size = 1.0/float(uResolution);
    
    vec2 uv = texel_size * vec2(texelCoord);
    
    //Get sun ray origin and direction:
    vec3 org = vec3(uv.x, getHeight(uv, 0.0), uv.y);

    vec3 dir = normalize(vec3(uScaleY * uSunDir.x,
                              uScaleXZ* uSunDir.y,
                              uScaleY * uSunDir.z));


    #ifdef FANCY
    //Fancy version

    float proj_fac = abs(dot(dir, vec3(1.0, 0.0, 1.0) * dir));

    vec3 dR = (texel_size/proj_fac) * dir;

    float J = 1.0, t = 1.0;
    int m = uMips - 1;

    for (int k=0; k<uMips-1; k++) {
        float min_t = -1.0, min_dh = 1.0;
        vec3 R = t * dR; //should be texel size for minimal mip, so multiplied by 2^(number of mips)

        float H = org.y + R.y;
        float h = getHeight(org.xz + R.xz, m);
        float dh = H - h;

        if (dh < min_dh) {
            min_dh = dh;
            min_t = t;
        }

        for (int j=1; j<=2; j++) {
            R = R - pow(2.0, -float(k)-1.0) * dR;
            t = t - pow(2.0, -float(k));

            H = org.y + R.y;
            h = getHeight(org.xz + R.xz, m);
            dh = H - h;

            if (dh < min_dh) {
                min_dh = dh;
                min_t = t;
            }
        }

        m--;
        
        if (min_t > -1.0) {
            t = min_t + pow(2.0, -float(k));
        }
    }

    #else
    //Naive Raymarch

    float dt = uMaxT/float(uSteps);
    float voffset = dt;
    org += vec3(0.0, voffset, 0.0);

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

    #endif

    imageStore(shadowmap, texelCoord, vec4(shadow, 0.0, 0.0, 1.0));
}
