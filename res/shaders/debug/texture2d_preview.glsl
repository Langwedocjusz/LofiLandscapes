#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D preview;

uniform sampler2D source;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/imageSize(preview);

    vec4 res = texture(source, uv);

    imageStore(preview, texelCoord, res);
}