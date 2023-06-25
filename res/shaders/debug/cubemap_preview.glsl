#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D preview;

uniform samplerCube source;

uniform int uSide;

#define POSITIVE_X 0
#define NEGATIVE_X 1
#define POSITIVE_Y 2
#define NEGATIVE_Y 3
#define POSITIVE_Z 4
#define NEGATIVE_Z 5

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 uv = 2.0 * vec2(texelCoord)/imageSize(preview) - 1.0;

    vec3 coord = vec3(0.0);

    switch(uSide) {
        case POSITIVE_X:
        {
            coord = vec3(1.0, uv.y, uv.x);
            break;
        }
        case NEGATIVE_X:
        {
            coord = vec3(-1.0, uv.y, -uv.x);
            break;
        }
        case POSITIVE_Y:
        {
            coord = vec3(uv.x, 1.0, uv.y);
            break;
        }
        case NEGATIVE_Y:
        {
            coord = vec3(-uv.x, -1.0, -uv.y);
            break;
        }
        case POSITIVE_Z:
        {
            coord = vec3(uv.x, uv.y, 1.0);
            break;
        }
        case NEGATIVE_Z:
        {
            coord = vec3(-uv.x, uv.y, -1.0);
            break;
        }
    }

    vec4 res = texture(source, coord);

    imageStore(preview, texelCoord, res);
}