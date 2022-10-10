#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform mat4 uMVP;

out vec3 col;

void main() {
    col = aCol;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
