#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform float uSlopeUpper;
uniform float uSlopeLower;
uniform float uBlend;
uniform int uID;

float getHeight(vec2 uv) {
    return texture(heightmap, uv).r;
}

vec3 getNorm(vec2 uv) {
    vec2 h = vec2(0.0, 1.0)/textureSize(heightmap, 0);

    return normalize(vec3(
        0.5*(getHeight(uv+h.yx) - getHeight(uv-h.yx))/h.y,
        1.0,
        0.5*(getHeight(uv+h.xy) - getHeight(uv-h.xy))/h.y
    ));
}

#define SUM_COMPONENTS(v) (v.x + v.y + v.z + v.w)

#define PI 3.1415926535

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    //Slope
    vec2 uv = vec2(texelCoord)/imageSize(materialmap);
    vec3 norm = getNorm(uv);

    float slope = (2.0/PI)*asin(1.0 - norm.y);

    //Mask
    float mask = smoothstep(uSlopeLower - uBlend, uSlopeLower + uBlend, slope)
        * (1.0 - smoothstep(uSlopeUpper - uBlend, uSlopeUpper + uBlend, slope));

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