#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aQuadSize;

layout (location = 3) in float aTrimFlag;

uniform mat4 uMVP;

uniform vec2 uPos;

void main() 
{
    vec2 hoffset = uPos - mod(uPos, aQuadSize);

    vec2 pos2 = aPos.xz;

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
    }

    vec3 pos = vec3(pos2.x, aPos.y, pos2.y) + vec3(hoffset.x, 0.0, hoffset.y);

    gl_Position = uMVP * vec4(pos, 1.0);
}
