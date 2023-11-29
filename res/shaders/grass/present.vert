#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aQuadSize;

layout (location = 3) in float aTrimFlag;

uniform float uScaleXZ;
uniform mat4 uMVP;

uniform vec3 uPos;

uniform float uGrassHeight;

out vec2 world_uv;

out vec3 frag_pos;

void main() {
    vec2 hoffset = uPos.xz - mod(uPos.xz, aQuadSize);
    vec3 offset = vec3(hoffset.x, uGrassHeight, hoffset.y);

    vec2 pos2 = aPos.xz;
    vec3 pos3 = aPos.xyz;

    if (aTrimFlag == 1.0)
    {
        ivec2 id2 = ivec2(hoffset/aQuadSize);

        id2.x = id2.x % 2;
        id2.y = id2.y % 2;

        int id = id2.x + 2*id2.y;

        mat2 rotations[4] = mat2[4](
            mat2(1.0, 0.0, 0.0, 1.0),
            mat2(0.0, 1.0, -1.0, 0.0),
            mat2(0.0, -1.0, 1.0, 0.0),
            mat2(-1.0, 0.0, 0.0, -1.0)
        );

        pos2 = rotations[id] * pos2;
        pos2 += aQuadSize * vec2(1-id2.x, 1-id2.y);

        pos3.x = pos2.x;
        pos3.z = pos2.y;
    }

    world_uv = (2.0/uScaleXZ) * (pos2 + hoffset);
    world_uv = 0.5*world_uv + 0.5;
    
    frag_pos = pos3 + offset;

    gl_Position = uMVP * vec4(pos3 + offset, 1.0);
}
