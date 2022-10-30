#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D tex;

uniform vec3 uCol1;
uniform vec3 uCol2;

void main() {
    float h = texture(tex, uv).w;
    vec3 col = mix(uCol1, uCol2, h);

    frag_col = vec4(col, 1.0);
}

