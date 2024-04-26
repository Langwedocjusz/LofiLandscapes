#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uScale;
uniform float uRandomness;

uniform int uVoronoiType;

#define VORONOI_F1        0
#define VORONOI_F2        1
#define VORONOI_F2_F1     2
#define VORONOI_F1_SMOOTH 3

uniform float uSmoothness;

uniform float uDistortion;
uniform int uOctaves;
uniform int uDistScale;
uniform float uRoughness;

#include "blending.glsl"
#include "common_fbm.glsl"

vec2 Voronoi(vec2 x){
    vec2 p = floor(x);
    vec2 q = fract(x);

    vec2 res = vec2(2.25);

    for (int i=-1; i<=1; i++) {
        for (int j=-1; j<=1; j++) {
            vec2 v = p + vec2(i,j);
            vec2 r = v + uRandomness*hash22(v, uScale);

            float d2 = dot(x-r, x-r);

            if (d2 < res.x) 
            {
                res.y = res.x;
                res.x = d2;
            }

            else if (d2 < res.y) 
            {
                res.t = d2;
            }
        }
    }

    return sqrt(res);
}

float SmoothVoronoi(vec2 x)
{
    float smoothing = max(0.01, pow(0.6*uSmoothness, 3.0));

    vec2 p = floor(x);
    vec2 q = fract(x);

    float res = 0.0;

    for (int i=-3; i<=3; i++) {
        for (int j=-3; j<=3; j++) {
            vec2 v = p + vec2(i,j);
            vec2 r = v + uRandomness*hash22(v, uScale);

            float d = length(x-r);

            res += exp2(-d/smoothing);
        }
    }

    return -smoothing*log2(res);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(heightmap);

    float prev = float(imageLoad(heightmap, texelCoord));

    float fnoise = fbm(float(uDistScale)*uv, uOctaves, uDistScale, uRoughness);
    uv = float(uScale)*uv + uDistortion*fnoise;

    vec2 voro = (uVoronoiType == VORONOI_F1_SMOOTH) 
              ? vec2(SmoothVoronoi(uv))
              : Voronoi(uv);

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
        case VORONOI_F1_SMOOTH:
        {
            h = voro.x;
            break;
        }
    }

    h = BlendResult(prev, h);

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}
