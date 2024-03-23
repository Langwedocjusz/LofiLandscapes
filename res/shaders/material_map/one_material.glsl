#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D materialmap;

uniform int uID;

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec4 res = vec4(0.0); //This is already case 4

    switch(uID) {
        case 0:
        {
            res = vec4(1,0,0,0);
            break;
        }
        case 1:
        {
            res = vec4(0,1,0,0);
            break;
        }
        case 2:
        {
            res = vec4(0,0,1,0);
            break;
        }
        case 3:
        {
            res = vec4(0,0,0,1);
            break;
        }
    }

    imageStore(materialmap, texelCoord, res);
}