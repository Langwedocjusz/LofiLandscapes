#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform float uCurvatureUpper;
uniform float uCurvatureLower;
uniform float uBlend;
uniform int uID;

float getHeight(vec2 uv) {
    return texture(heightmap, uv).r;
}

float getCurvature(vec2 uv) {
    vec2 h = vec2(0.0, 1.0)/textureSize(heightmap, 0);

    float f = getHeight(uv);

    float ddx = getHeight(uv + h.yx) + getHeight(uv - h.yx)
              - 2.0 * f;

    float ddy = getHeight(uv + h.xy) + getHeight(uv - h.xy)
              - 2.0 * f;

    return (ddx + ddy);
}

#define SUM_COMPONENTS(v) (v.x + v.y + v.z + v.w)

#define PI 3.1415926535

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    //Slope
    vec2 uv = vec2(texelCoord)/imageSize(materialmap);

    const float sensitivity = 100.0;

    float curvature = atan(sensitivity * getCurvature(uv))/PI + 0.5;

    //Mask
    float mask = smoothstep(uCurvatureLower - uBlend, uCurvatureLower + uBlend, curvature)
        * (1.0 - smoothstep(uCurvatureUpper - uBlend, uCurvatureUpper + uBlend, curvature));

    //Previous
    vec4 prev03 = imageLoad(materialmap, texelCoord);
    float prev4 = 1.0 - SUM_COMPONENTS(prev03);

    float prev[5] = float[5](prev03.x, prev03.y, prev03.z, prev03.w, prev4);

    //Overlay
    float sum = 0.0;

    for (int i=0; i<5; i++)
    {
        if (i != uID)
            sum += prev[i];
    }

    for (int i=0; i<5; i++)
    {
        if (i == uID)
        {
            prev[i] = mask;
        }
        else
        {
            prev[i] = (1-mask)*prev[i]/sum;
        }
    }

    vec4 res = vec4(prev[0], prev[1], prev[2], prev[3]);

    imageStore(materialmap, texelCoord, res);
}