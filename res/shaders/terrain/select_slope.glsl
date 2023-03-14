#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r8, binding = 0) uniform image2D materialmap;

uniform sampler2D heightmap;

uniform int uResolution;
uniform float uSlopeUpper;
uniform float uSlopeLower;
uniform int uID;

float getHeight(vec2 uv) {
    return texture(heightmap, uv).r;
}

vec3 getNorm(vec2 uv) {
    vec2 h = vec2(0.0, 1.0/float(uResolution));

    return normalize(vec3(
        0.5*(getHeight(uv+h.yx) - getHeight(uv-h.yx))/h.y,
        1.0,
        0.5*(getHeight(uv+h.xy) - getHeight(uv-h.xy))/h.y
    ));
}

const float MAX_ID = 3.0;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    float prev = imageLoad(materialmap, texelCoord).r;

    vec2 uv = vec2(texelCoord)/float(uResolution);

    vec3 norm = getNorm(uv);
    float slope = norm.y;

    float res = prev;

    if (slope <= uSlopeUpper && slope >= uSlopeLower)
        res = float(uID)/MAX_ID;

    imageStore(materialmap, texelCoord, vec4(res, 0.0, 0.0, 1.0));
}