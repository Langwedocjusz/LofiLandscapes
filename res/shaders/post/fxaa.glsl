//FXAA antialiasing implementation based on the following:
//https://catlikecoding.com/unity/tutorials/advanced-rendering/fxaa/

#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D OutputImage;

uniform sampler2D InputTexture;

uniform float uContrastThreshold;
uniform float uRelativeThreshold;
uniform float uSubpixelAmount;

vec2 getUV(ivec2 texel_coord)
{
    return (vec2(texel_coord)+0.5)/imageSize(OutputImage);
}

void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = getUV(texelCoord);

    vec4 color_and_luma = texture(InputTexture, uv);

    vec3 color = color_and_luma.rgb;
    float luma = color_and_luma.a;

    float luma_n = texture(InputTexture, getUV(texelCoord + ivec2( 0, 1))).a;
    float luma_s = texture(InputTexture, getUV(texelCoord + ivec2( 0,-1))).a;
    float luma_w = texture(InputTexture, getUV(texelCoord + ivec2(-1, 0))).a;
    float luma_e = texture(InputTexture, getUV(texelCoord + ivec2( 1, 0))).a;

    float luma_ne = texture(InputTexture, getUV(texelCoord + ivec2(-1, 1))).a;
    float luma_nw = texture(InputTexture, getUV(texelCoord + ivec2( 1, 1))).a;
    float luma_sw = texture(InputTexture, getUV(texelCoord + ivec2(-1,-1))).a;
    float luma_se = texture(InputTexture, getUV(texelCoord + ivec2( 1,-1))).a;

    //Local contrast computation
    float min_luma = min(luma, min(luma_n, min(luma_s, min(luma_w, luma_e))));
    float max_luma = max(luma, max(luma_n, max(luma_s, max(luma_w, luma_e))));

    float local_contrast = max_luma - min_luma;

    //Skip pixels if local contrast is too low
    bool skip_pixel = (local_contrast < max(uContrastThreshold, uRelativeThreshold * max_luma));

    if (skip_pixel)
    {
        vec4 res = vec4(color, 1.0);
        imageStore(OutputImage, texelCoord, res);
        return;
    }

    //Blend factor computation
    float blend_factor = 2.0 * (luma_n + luma_s + luma_w + luma_e);
    blend_factor += (luma_ne + luma_nw + luma_se + luma_sw);

    blend_factor /= 12.0;
    blend_factor = abs(blend_factor - color_and_luma.a);
    blend_factor = clamp(blend_factor / local_contrast, 0.0, 1.0);

    blend_factor = smoothstep(0.0, 1.0, blend_factor);
	blend_factor *= blend_factor;
    blend_factor *= uSubpixelAmount;

    //To-do:
    //Implement better edge detection with multi-step search

    //Edge type detection
    float h_weight = 2.0 * abs(luma_n + luma_s - 2.0 * luma) 
                   + abs(luma_ne + luma_se - 2.0 * luma_e) 
                   + abs(luma_nw + luma_sw - 2.0 * luma_w);

    float v_weight = 2.0 * abs(luma_e + luma_w - 2.0 * luma)
				   + abs(luma_ne + luma_nw - 2.0 * luma_n)
				   + abs(luma_se + luma_sw - 2.0 * luma_s);

    bool edge_horizontal = (h_weight >= v_weight);

    //Gradient sign determination
    float pos_luma = edge_horizontal ? luma_n : luma_e;
    float neg_luma = edge_horizontal ? luma_s : luma_w;

    float pos_grad = abs(pos_luma - luma);
    float neg_grad = abs(neg_luma - luma);

    float step_size = edge_horizontal 
               ? 1.0/imageSize(OutputImage).y
               : 1.0/imageSize(OutputImage).x;

    if (pos_grad < neg_grad)
        step_size *= -1.0;

    //Sampling input texture at adjusted location
    vec2 new_uv = uv;

    if (edge_horizontal)
        new_uv.y += step_size * blend_factor;
    else
        new_uv.y += step_size * blend_factor;

    color = textureLod(InputTexture, new_uv, 0.0).rgb;

    vec4 res = vec4(vec3(color), 1.0);

    imageStore(OutputImage, texelCoord, res);
}