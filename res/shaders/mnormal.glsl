#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D normalmap;

uniform sampler2D heightmap;

uniform int uResolution;

float getHeight(vec2 uv) {
    return texture(heightmap, uv).r;
}

vec3 getNorm(vec2 uv) {
    vec2 h = vec2(0.0, 1.0/512.0);

    return normalize(vec3(
        0.5*(getHeight(uv+h.yx) - getHeight(uv-h.yx))/h.y,
        1.0,
        0.5*(getHeight(uv+h.xy) - getHeight(uv-h.xy))/h.y
    ));
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    vec3 norm = 0.5*getNorm(uv)+0.5;

    imageStore(normalmap, texelCoord, vec4(norm, 1.0));
}
