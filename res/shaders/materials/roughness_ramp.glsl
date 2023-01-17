#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform sampler2D heightmap;

uniform int uResolution;

uniform float uEdge1;
uniform float uEdge2;

uniform float uVal1;
uniform float uVal2;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 prev = imageLoad(albedo, texelCoord);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    float h = texture(heightmap, uv).r;

    float fac = saturate((h-uEdge1)/(uEdge2-uEdge1));

    float val = mix(uVal1, uVal2, fac);

    vec4 res = vec4(prev.rgb, val);

    imageStore(albedo, texelCoord, res);
}
