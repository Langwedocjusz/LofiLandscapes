#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D albedo;

uniform int uResolution;

uniform sampler2D tex;

uniform vec3 uCol1;
uniform vec3 uCol2;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = vec2(texelCoord)/float(uResolution);
    
    float h = texture(tex, uv).w;
    vec3 col = mix(uCol1, uCol2, h);

    imageStore(albedo, texelCoord, vec4(col, 1.0));
}
