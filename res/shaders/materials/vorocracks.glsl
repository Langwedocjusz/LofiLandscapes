#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 0) uniform image2D heightmap;

uniform int uScale;
uniform float uRandomness;
uniform float uThickness;

uniform float uDistortion;
uniform int uOctaves;
uniform int uDistScale;
uniform float uRoughness;

#include "blending.glsl"
#include "common_fbm.glsl"

//Distanse to voronoi edges by Inigo Quilez:
//https://iquilezles.org/articles/voronoilines/
float VoroLinesDist(vec2 x)
{
    ivec2 p = ivec2(floor(x));
    vec2  f = fract(x);

    ivec2 mb;
    vec2 mr;

    float res = 8.0;

    for( int j=-1; j<=1; j++ )
    {
        for( int i=-1; i<=1; i++ )
        {
            ivec2 b = ivec2(i, j);
            vec2  r = vec2(b) + uRandomness*hash22(p+b, uScale)-f;
            float d = dot(r,r);

            if( d < res )
            {
                res = d;
                mr = r;
                mb = b;
            }
        }
    }

    res = 8.0;
    
    for(int j=-2; j<=2; j++)
    {
        for(int i=-2; i<=2; i++)
        {
            ivec2 b = mb + ivec2(i, j);
            vec2  r = vec2(b) + uRandomness*hash22(p+b, uScale) - f;
            float d = dot(0.5*(mr+r), normalize(r-mr));

            res = min( res, d );
        }
    }

    return res;
}

float VoroLines(vec2 p)
{
    float dist = VoroLinesDist(p);

    return 1.0 - smoothstep(0.0,uThickness,dist);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoord)/imageSize(heightmap);

    float prev = float(imageLoad(heightmap, texelCoord));

    float fnoise = fbm(float(uDistScale)*uv, uOctaves, uDistScale, uRoughness);
    uv = float(uScale)*uv + uDistortion*fnoise;

    float h = VoroLines(uv);

    h = BlendResult(prev, h);

    vec4 res = vec4(h, vec3(0.0));

    imageStore(heightmap, texelCoord, res);
}
