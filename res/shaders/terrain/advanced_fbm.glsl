//Based on this article by Inigo Quilez:
//https://iquilezles.org/articles/morenoise/
//Also referenced this paper:
//https://www.sbgames.org/sbgames2018/files/papers/ComputacaoShort/188264.pdf

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

uniform int uOctaves;
uniform float uScale;

uniform float uRoughness;
uniform float uLacunarity;
uniform float uAltitudeErosion;
uniform float uSlopeErosion;

uniform int uBlendMode;

#define BLEND_AVERAGE  0
#define BLEND_ADD      1
#define BLEND_SUBTRACT 2

uniform float uWeight;

//High-quality hash function by nojima:
//https://www.shadertoy.com/view/ttc3zr
uint murmurHash12(uvec2 src) {
    const uint M = 0x5bd1e995u;
    uint h = 1190494759u;
    src *= M; src ^= src>>24u; src *= M;
    h *= M; h ^= src.x; h *= M; h ^= src.y;
    h ^= h>>13u; h *= M; h ^= h>>15u;
    return h;
}

float hash(vec2 src) {
    uint h = murmurHash12(floatBitsToUint(src));
    return uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

vec3 noised(vec2 p) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a = hash(id+vec2(0,0));
    float b = hash(id+vec2(1,0));
    float c = hash(id+vec2(0,1));
    float d = hash(id+vec2(1,1));

    u = u*u*u*(u*(6.0*u-15.0)+10.0);

    vec2 du = 30.0*u*u*(u*u-2.0*u+1.0);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    return vec3(
        k0 + k1*u.x + k2*u.y + k3*u.x*u.y,
        vec2(k1 + k3*u.y, k2 + k3*u.x) * du
    );
}

float fbm(in vec2 p, int octaves, float prev_height) {
    const float scale_y = 1.0;
    const float scale_xz = 0.5;
    const mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);

    p *= scale_xz;

    mat2 M = mat2(1.0);
    float A = 1.0, a = 1.0;

    float value = 0.0, normalization = 0.0;
    vec2 grad = vec2(0.0);

    for (int i=0; i<octaves; i++) {
        const vec3 n = noised(a*M*p);
        
        //This can also be treated differently depending on the uBlendMode,
        //but it's debatable if it makes the behaviour more natural
        float actual_height = value + prev_height;

        float xi = (i==0) ? A : mix(A, A/(1.0 + dot(grad, grad)), uSlopeErosion);
        xi = (i==0) ? A : mix(xi, xi*max(0.0, actual_height), uAltitudeErosion);

        value += xi*n.x;
        grad += xi*a*M*n.yz;
        normalization += A;

        a *= uLacunarity;
        A *= uRoughness;
        M *= rot;
    }

    return scale_y * value/normalization;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = float(imageLoad(heightmap, texelCoord));

    vec2 uv = vec2(texelCoord)/imageSize(heightmap);
    vec2 ts = uScale*uv;

    float h = fbm(ts, uOctaves, prev);

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
