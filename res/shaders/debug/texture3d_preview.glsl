#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D preview;

uniform sampler3D source;

uniform float uDepth;

uniform vec2 uRange;

vec4 RemapFrom(vec4 value, vec2 range) {
    return (value - vec4(range.x))/(range.y - range.x);
}

uniform vec4 uChannelFlags;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/imageSize(preview);
    uv.y = 1.0 - uv.y;

    vec4 res = texture(source, vec3(uv, uDepth));

    res = RemapFrom(res, uRange);

    res *= uChannelFlags;

    if (uChannelFlags.a == 0.0)
        res.a = 1.0;

    else if (uChannelFlags.rgb == vec3(0.0))
    {
        res.rgb = vec3(res.a);
        res.a = 1.0;
    }

    imageStore(preview, texelCoord, res);
}