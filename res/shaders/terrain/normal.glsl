#version 450 core

#define PI 3.1415926535
#define GOLDEN_RATIO 1.6180339887

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D normalmap;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;

uniform int uAOSamples;
uniform float uAOR;

float getHeight(vec2 p) {
    return texture(heightmap, p).r;
}

vec3 getNorm(vec2 p) {
    vec2 h = vec2(0.0, 1.0)/textureSize(heightmap, 0);

    return normalize(vec3(
        uScaleY*0.5*(getHeight(p+h.yx) - getHeight(p-h.yx))/h.y,
        uScaleXZ,
        uScaleY*0.5*(getHeight(p+h.xy) - getHeight(p-h.xy))/h.y
    ));
}

float getAO(vec2 p) {
    const float dt = 1.0/float(uAOSamples);
    const float A = 2*PI*float(uAOSamples)/GOLDEN_RATIO;
    const float h_0 = getHeight(p);
    const float C = uScaleY/uScaleXZ;
    
    float res = 0.0;

    for (float t=0.0; t<1.0; t+=dt) {
        float radius = uAOR*sqrt(t);
        if (radius <= 0.0f) continue;

        vec2 q = p + radius*vec2(sin(A*t), cos(A*t));

        float h = getHeight(q);
        float rel_h = max(h-h_0, 0.0);
        float theta = 0.5*PI - atan(C*rel_h/radius);
        float dres = 1.0 - cos(theta);
        
        res += dres;
    }

    res = res/float(uAOSamples);
    
    return res;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/imageSize(normalmap);
    
    float ao = getAO(uv);
    vec3 norm = 0.5*getNorm(uv) + 0.5;

    imageStore(normalmap, texelCoord, vec4(norm, ao));
}
