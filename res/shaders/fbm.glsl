#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uResolution;

uniform int uOctaves;
uniform int uScale;

uniform int uBlendMode;

#define BLEND_AVERAGE  0
#define BLEND_ADD      1
#define BLEND_SUBTRACT 2

uniform float uWeight;

float hash(vec2 p, float scale) {
    p = (scale == 0.0) ? vec2(0.0) : mod(p, scale);
    if (p == vec2(0.0)) return 0.5;
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p, float scale) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a = hash(id+vec2(0,0), scale);
    float b = hash(id+vec2(1,0), scale);
    float c = hash(id+vec2(0,1), scale);
    float d = hash(id+vec2(1,1), scale);

    u = u*u*(3.0-2.0*u);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(in vec2 p, int octaves) {
    const float normalization = 1.0/(2.0 - pow(0.5, float(octaves)));

    float res = 0.0;

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*p, float(uScale)*a);

        a *= 2.0;
        A *= 0.5;
    }

    return normalization * res;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = float(imageLoad(heightmap, texelCoord));

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
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
