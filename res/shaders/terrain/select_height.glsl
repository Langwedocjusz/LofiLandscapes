#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform int uResolution;
uniform float uHeightUpper;
uniform float uHeightLower;
uniform int uID;

const float MAX_ID = 3.0;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = imageLoad(materialmap, texelCoord).r;

    vec2 uv = vec2(texelCoord)/float(uResolution);

    float height = texture(heightmap, uv).r;

    float res = prev;

    if (height <= uHeightUpper && height >= uHeightLower)
        res = float(uID)/MAX_ID;

    imageStore(materialmap, texelCoord, vec4(res, 0.0, 0.0, 1.0));
}