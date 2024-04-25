#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uScale;
uniform float uRandomness;

uniform int uVoronoiType;

#define VORONOI_F1    0
#define VORONOI_F2    1
#define VORONOI_F2_F1 2

#include "../common/hash.glsl"
#include "blending.glsl"

vec2 voronoi(vec2 x){
    vec2 p = floor(x);
    vec2 q = fract(x);

    vec2 res = vec2(2.25);

    for (int i=-1; i<=1; i++) {
        for (int j=-1; j<=1; j++) {
            vec2 v = p + vec2(i,j);
            vec2 r = v + uRandomness*hash22(v, uScale);

            float d2 = dot(x-r, x-r);

            if (d2 < res.x) {
                res.y = res.x;
                res.x = d2;
            }

            else if (d2 < res.y) {
                res.t = d2;
            }
        }
    }

    return sqrt(res);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(heightmap);

    float prev = float(imageLoad(heightmap, texelCoord));

    vec2 voro = voronoi(float(uScale)*uv);
    float h = 0.0;

    switch(uVoronoiType) {
        case VORONOI_F1:
        {
            h = voro.x;
            break;
        }
        case VORONOI_F2:
        {
            h = voro.y;
            break;
        }
        case VORONOI_F2_F1:
        {
            h = voro.y - voro.x;
            break;
        }
    }

    h = BlendResult(prev, h);

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}
