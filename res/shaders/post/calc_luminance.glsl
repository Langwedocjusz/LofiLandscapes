#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D OutputImage;

uniform sampler2D InputTexture;

void main()
{
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = (vec2(texelCoord)+0.5)/imageSize(OutputImage);

    vec3 color = texture(InputTexture, uv).rgb;

    float luminance = dot(color, vec3(0.2126729,  0.7151522, 0.0721750));

    vec4 res = vec4(color, luminance);

    imageStore(OutputImage, texelCoord, res);
}