#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

uniform int uOctaves;
uniform float uScale;

uniform float uRoughness;

uniform int uBlendMode;

#define BLEND_AVERAGE  0
#define BLEND_ADD      1
#define BLEND_SUBTRACT 2

uniform float uWeight;

#include "../common/hash.glsl"

float noise(vec2 p) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a = hash12(id+vec2(0,0));
    float b = hash12(id+vec2(1,0));
    float c = hash12(id+vec2(0,1));
    float d = hash12(id+vec2(1,1));

    u = u*u*u*(u*(6.0*u-15.0)+10.0);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
}

float fbm(in vec2 p, int octaves) {
    const float scale_y = 1.0;
    const float scale_xz = 0.5;
    const mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);

    p *= scale_xz;

    float res = 0.0;
    mat2 M = mat2(1.0);

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*M*p);

        a *= 2.0;
        A *= uRoughness;
        M *= rot;
    }

    return scale_y * res;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = float(imageLoad(heightmap, texelCoord));

    vec2 uv = vec2(texelCoord)/imageSize(heightmap);
    vec2 ts = uScale*uv;

    float h = fbm(ts, uOctaves);

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

    imageStore(heightmap, texelCoord, vec4(h));
}
