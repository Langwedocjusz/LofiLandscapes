#version 450 core

#define PI           3.1415926535
#define GOLDEN_RATIO 1.6180339887

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D normalmap;

uniform sampler2D heightmap;

uniform int uResolution;

uniform float uAOStrength;
uniform float uAOSpread;
uniform float uAOContrast;

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

float getAO(vec2 p) {
    vec2 h = vec2(0.0, uAOSpread/float(uResolution));

    float cx = max(getNorm(p+h.xy).z, 0.0) + max(-getNorm(p-h.yx).x, 0.0);
    float cz = max(getNorm(p+h.xy).z, 0.0) + max(-getNorm(p-h.xy).z, 0.0);

    float res = 1.0 - uAOContrast*0.25*(cx+cz);

    return clamp(res, 0.0, 1.0);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    vec3 norm = getNorm(uv);
    norm = 0.5*getNorm(uv)+0.5;

    float ao = smoothstep(-uAOStrength, 1.0, getAO(uv));

    imageStore(normalmap, texelCoord, vec4(norm, ao));
}
