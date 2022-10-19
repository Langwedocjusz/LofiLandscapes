#version 450 core

layout (location = 0) in vec4 aPos;

uniform float uL;
uniform mat4 uMVP;

uniform vec2 uPos;

uniform sampler2D tex;

void main() {
    vec2 hoffset = uPos - mod(uPos, aPos.w);
    
    gl_Position = uMVP * vec4(aPos.xyz + vec3(hoffset.x, 0.0, hoffset.y), 1.0);
}
