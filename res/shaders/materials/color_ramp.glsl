#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform sampler2D heightmap;

uniform int uResolution;

uniform float uEdge1;
uniform float uEdge2;

uniform vec3 uCol1;
uniform vec3 uCol2;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    float h = texture(heightmap, uv).r;

    float fac = saturate((h-uEdge1)/(uEdge2-uEdge1));

    vec3 col = mix(uCol1, uCol2, fac);

    imageStore(albedo, texelCoord, vec4(col, 1.0));
}
