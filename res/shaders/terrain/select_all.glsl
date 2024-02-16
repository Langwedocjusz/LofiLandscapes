#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform float uHeightUpper;
uniform float uHeightLower;
uniform float uSlopeUpper;
uniform float uSlopeLower;
uniform float uCurvatureUpper;
uniform float uCurvatureLower;

uniform float uBlend;
uniform int uID;

#define PI 3.1415926535

#define SUM_COMPONENTS(v) (v.x + v.y + v.z + v.w)

float getHeight(vec2 uv) {
    return texture(heightmap, uv).r;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    vec2 uv = vec2(texelCoord)/imageSize(materialmap);

    vec2 h = vec2(0.0, 1.0)/textureSize(heightmap, 0);

    float height = getHeight(uv);

    float up = getHeight(uv+h.xy);
    float down = getHeight(uv-h.xy);
    float right = getHeight(uv+h.yx);
    float left = getHeight(uv-h.yx);

    vec3 norm = normalize(vec3(
        0.5*(right - left)/h.y,
        1.0,
        0.5*(up - down)/h.y
    ));

    float slope = (2.0/PI)*asin(1.0 - norm.y);

    float ddx = right + left - 2.0 * height;
    float ddy = up + down - 2.0 * height;

    const float sensitivity = 100.0;
    float curvature = atan(sensitivity * (ddx + ddy))/PI + 0.5;

    float height_mask = smoothstep(uHeightLower - uBlend, uHeightLower + uBlend, height)
        * (1.0 - smoothstep(uHeightUpper - uBlend, uHeightUpper + uBlend, height));

    float slope_mask = smoothstep(uSlopeLower - uBlend, uSlopeLower + uBlend, slope)
        * (1.0 - smoothstep(uSlopeUpper - uBlend, uSlopeUpper + uBlend, slope));

    float curvature_mask = smoothstep(uCurvatureLower - uBlend, uCurvatureLower + uBlend, curvature)
        * (1.0 - smoothstep(uCurvatureUpper - uBlend, uCurvatureUpper + uBlend, curvature));

    float mask = height_mask * slope_mask * curvature_mask;

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
