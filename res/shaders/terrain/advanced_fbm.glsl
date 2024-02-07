//Based on this article by Inigo Quilez:
//https://iquilezles.org/articles/morenoise/
//Also referenced this paper:
//https://www.sbgames.org/sbgames2018/files/papers/ComputacaoShort/188264.pdf

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r32f, binding = 0) uniform image2D heightmap;

uniform int uNoiseType;

#define NOISE_VALUE 0
#define NOISE_PERLIN 1

uniform int uOctaves;
uniform float uScale;

uniform float uRoughness;
uniform float uLacunarity;
uniform float uAltitudeErosion;
uniform float uSlopeErosion;
uniform float uConcaveErosion;

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

float hash12(vec2 src) {
    uint h = murmurHash12(floatBitsToUint(src));
    return uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

uvec2 murmurHash22(uvec2 src) {
    const uint M = 0x5bd1e995u;
    uvec2 h = uvec2(1190494759u, 2147483647u);
    src *= M; src ^= src>>24u; src *= M;
    h *= M; h ^= src.x; h *= M; h ^= src.y;
    h ^= h>>13u; h *= M; h ^= h>>15u;
    return h;
}

vec2 hash22(vec2 src) {
    uvec2 h = murmurHash22(floatBitsToUint(src));
    return uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

//Returns noise function (value, [gradient], laplacian)
vec4 noised(vec2 p) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a,b,c,d;
    vec2 va, vb, vc, vd;

    if (uNoiseType == NOISE_PERLIN)
    {
        va = 2.0*hash22(id+vec2(0,0)) - 1.0;
        vb = 2.0*hash22(id+vec2(1,0)) - 1.0;
        vc = 2.0*hash22(id+vec2(0,1)) - 1.0;
        vd = 2.0*hash22(id+vec2(1,1)) - 1.0;

        a = 0.5*dot(va, u - vec2(0,0)) + 0.5;
        b = 0.5*dot(vb, u - vec2(1,0)) + 0.5;
        c = 0.5*dot(vc, u - vec2(0,1)) + 0.5;
        d = 0.5*dot(vd, u - vec2(1,1)) + 0.5;
    }
    
    else
    {
        a = hash12(id+vec2(0,0));
        b = hash12(id+vec2(1,0));
        c = hash12(id+vec2(0,1));
        d = hash12(id+vec2(1,1));
    }

    u = u*u*u*(u*(6.0*u-15.0)+10.0);
    vec2 du = 30.0*u*u*(u*u-2.0*u+1.0);
    vec2 ddu = 60.0*u*(2.0*u*u - 3.0*u + 1.0);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    if (uNoiseType == NOISE_PERLIN)
    {
        vec2 vk0 = va;
        vec2 vk1 = vb - va;
        vec2 vk2 = vc - va;
        vec2 vk3 = va - vb - vc + vd;

        return vec4(
            k0 + k1*u.x + k2*u.y + k3*u.x*u.y,

            vec2(vk0.x + vk1.x*u.x + vk2.x*u.y + vk3.x*u.x*u.y, vk0.y + vk1.y*u.x + vk2.y*u.y + vk3.y*u.x*u.y) 
                + vec2(k1 + k3*u.y, k2 + k3*u.x) * du,

            dot(vec2(1,1), (
                vec2(vk1.x + vk3.x*u.y, vk2.y + vk3.y*u.x) * du
                + vec2(vk1.x + vk3.x*u.y, vk2.y + vk3.y*u.x) * du
                + vec2(k1 + k3*u.y, k2 + k3*u.x) * ddu
            ))
        );
    }
    
    else
    {
        return vec4(
            k0 + k1*u.x + k2*u.y + k3*u.x*u.y,
            vec2(k1 + k3*u.y, k2 + k3*u.x) * du,
            dot(vec2(1,1), (vec2(k1 + k3*u.y, k2 + k3*u.x) * ddu))
        );
    }
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
    float laplacian = 0.0;

    for (int i=0; i<octaves; i++) {
        const vec4 n = noised(a*M*p);
        
        //This can also be treated differently depending on the uBlendMode,
        //but it's debatable if it makes the behaviour more natural
        float actual_height = value + prev_height;

        float xi = (i==0) ? A : mix(A, A/(1.0 + dot(grad, grad)), uSlopeErosion);
        xi = (i==0) ? A : mix(xi, xi*max(0.0, actual_height), uAltitudeErosion);
        xi = (i==0) ? A : mix(xi, xi/(1.0 + abs(min(0.5*laplacian, 0))), uConcaveErosion);

        value += xi*n.x;
        grad += xi*a*M*n.yz;
        laplacian += xi*a*a*n.w;

        normalization += A;

        a *= uLacunarity;
        A *= uRoughness;

        if (uNoiseType == NOISE_VALUE)
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
