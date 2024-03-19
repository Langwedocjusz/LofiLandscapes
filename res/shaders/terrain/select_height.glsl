#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform float uHeightUpper;
uniform float uHeightLower;
uniform float uBlend;
uniform int uID;

#define SUM_COMPONENTS(v) (v.x + v.y + v.z + v.w)

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    //Height
    vec2 uv = vec2(texelCoord)/imageSize(materialmap);
    float height = texture(heightmap, uv).r;

    //Mask
    float mask = smoothstep(uHeightLower - uBlend, uHeightLower + uBlend, height)
        * (1.0 - smoothstep(uHeightUpper - uBlend, uHeightUpper + uBlend, height));

    //Previous
    vec4 prev03 = imageLoad(materialmap, texelCoord);
    float prev4 = 1.0 - SUM_COMPONENTS(prev03);

    float prev[5] = float[5](prev03.x, prev03.y, prev03.z, prev03.w, prev4);

    //Overlay
    if (mask > 0.0)
    {
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
    }

    vec4 res = vec4(prev[0], prev[1], prev[2], prev[3]);

    imageStore(materialmap, texelCoord, res);
}