#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D framebuffer;

void main()
{
    frag_col = texture(framebuffer, uv);
}