#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uOctaves;
uniform int uScale;
uniform float uRoughness;

uniform int uBlendMode;

#define BLEND_AVERAGE  0
#define BLEND_ADD      1
#define BLEND_SUBTRACT 2

uniform float uWeight;

#include "../common/hash.glsl"

float noise(vec2 p, float scale) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    vec2 va = 2.0*hash22(id+vec2(0,0), scale) - 1.0;
    vec2 vb = 2.0*hash22(id+vec2(1,0), scale) - 1.0;
    vec2 vc = 2.0*hash22(id+vec2(0,1), scale) - 1.0;
    vec2 vd = 2.0*hash22(id+vec2(1,1), scale) - 1.0;

    float a = dot(va, u - vec2(0,0)) + 0.5;
    float b = dot(vb, u - vec2(1,0)) + 0.5;
    float c = dot(vc, u - vec2(0,1)) + 0.5;
    float d = dot(vd, u - vec2(1,1)) + 0.5;

    u = u*u*(3.0-2.0*u);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
}

float fbm(in vec2 p, int octaves) {
    float normalization = 0.0;
    float res = 0.0;

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*p, float(uScale)*a);
        normalization += A;

        a *= 2.0;
        A *= uRoughness;
    }

    return res/normalization;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = float(imageLoad(heightmap, texelCoord));

    vec2 uv = vec2(texelCoord)/imageSize(heightmap);
    
    float h = fbm(float(uScale)*uv, uOctaves);
    h = clamp(h, 0.0, 1.0);

    switch(uBlendMode) {
        case BLEND_AVERAGE:
        {
            h = mix(prev, h, uWeight);
            break;
        }
        case BLEND_ADD:
        {
            h = prev + uWeight*h;
            break;
        }
        case BLEND_SUBTRACT:
        {
            h = prev - uWeight*h;
            break;
        }
    }

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}
