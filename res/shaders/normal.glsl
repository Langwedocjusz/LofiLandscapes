#version 450 core

#define PI 3.1415926535

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D normalmap;

uniform int uResolution;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;

float getHeight(vec2 p) {
    return texture(heightmap, p).r;
}

vec3 getNorm(vec2 p) {
    vec2 h = vec2(0.0, 1.0/float(uResolution));

    return normalize(vec3(
        0.5*(getHeight(p+h.yx) - getHeight(p-h.yx))/h.y,
        uScaleXZ/uScaleY,
        0.5*(getHeight(p+h.xy) - getHeight(p-h.xy))/h.y
    ));
}

float getAO(vec2 p) {
    const int samples = 10;
    const float R = 0.0001;
    
    float h_0 = getHeight(p);
    float res = 0.0;

    for (int i=0; i<samples; i++) {
        float h = getHeight(p + R*vec2(cos(i*2.0*PI/float(samples)), 
                                       sin(i*2.0*PI/float(samples))));
        h = max(h-h_0, 0.0);

        float theta = 0.5*PI - atan(uScaleY*h/(uScaleXZ*R));
        res += 2*PI*R*(1.0-cos(theta));
    }

    return (res/float(samples))/(2.0*PI*R);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    float ao = getAO(uv);
    vec3 norm = 0.5*getNorm(uv) + 0.5;

    imageStore(normalmap, texelCoord, vec4(norm, ao));
}
