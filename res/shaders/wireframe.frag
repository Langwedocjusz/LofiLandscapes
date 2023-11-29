#version 450 core

out vec4 frag_col;

uniform vec3 uCol;

void main() {
    frag_col = vec4(uCol, 1.0);
}
