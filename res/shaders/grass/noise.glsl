#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D noisemap;

uniform int uScale;
uniform int uOctaves;

//High-quality hash function by nojima:
//https://www.shadertoy.com/view/ttc3zr
uvec2 murmurHash22(uvec2 src) {
    const uint M = 0x5bd1e995u;
    uvec2 h = uvec2(1190494759u, 2147483647u);
    src *= M; src ^= src>>24u; src *= M;
    h *= M; h ^= src.x; h *= M; h ^= src.y;
    h ^= h>>13u; h *= M; h ^= h>>15u;
    return h;
}

vec2 hash22(vec2 src, float scale) {
    src = mod(src, scale);

    uvec2 h = murmurHash22(floatBitsToUint(src));
    return uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
}

vec2 noise(vec2 p, float scale) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    vec2 a = hash22(id+vec2(0,0), scale);
    vec2 b = hash22(id+vec2(1,0), scale);
    vec2 c = hash22(id+vec2(0,1), scale);
    vec2 d = hash22(id+vec2(1,1), scale);

    u = u*u*(3.0-2.0*u);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

vec2 fbm(in vec2 p, int octaves) {
    float normalization = 0.0;
    vec2 res = vec2(0.0);

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*p, float(uScale)*a);
        normalization += A;

        a *= 2.0;
        A *= 0.5;
    }

    return res/normalization;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(noisemap);

    vec2 offset = fbm(float(uScale)*uv, uOctaves);

    vec3 norm = vec3(offset.x, 0.0, offset.y);

    vec4 res = vec4(norm, 1.0);

    imageStore(noisemap, texelCoord, res);
}