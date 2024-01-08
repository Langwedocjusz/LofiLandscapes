#version 450 core

in vec3 EdgeColor;

out vec4 frag_col;

uniform vec3 uCol;

void main() {
    //#define VISUALIZE_EDGE_FLAGS

    #ifdef VISUALIZE_EDGE_FLAGS
    frag_col = vec4(EdgeColor, 1.0);
    #else
    frag_col = vec4(uCol, 1.0);
    #endif
}
