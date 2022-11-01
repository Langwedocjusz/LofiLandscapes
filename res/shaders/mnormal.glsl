#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D normalmap;

uniform int uResolution;

float hash(vec2 p, float scale) {
    p = mod(p, scale);
    if (p == vec2(0.0)) return 0.5;
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p, float scale) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a = hash(id+vec2(0,0), scale);
    float b = hash(id+vec2(1,0), scale);
    float c = hash(id+vec2(0,1), scale);
    float d = hash(id+vec2(1,1), scale);

    u = u*u*(3.0-2.0*u);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float fbm(in vec2 p, int octaves) {
    const float scale_y = 1.0;

    float res = 0.0;

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*p, a);

        a *= 2.0;
        A *= 0.5;
    }

    return scale_y * res;
}

float getHeight(vec2 p) {
    return fbm(4.0*p, 8);
}

vec3 getNorm(vec2 p) {
    vec2 h = vec2(0.0, 1.0/512.0);

    return normalize(vec3(
        0.5*(getHeight(p+h.yx) - getHeight(p-h.yx))/h.y,
        1.0,
        0.5*(getHeight(p+h.xy) - getHeight(p-h.xy))/h.y
    ));
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    float h = getHeight(uv);
    vec3 norm = 0.5*getNorm(uv)+0.5;

    imageStore(normalmap, texelCoord, vec4(norm, h));
}
