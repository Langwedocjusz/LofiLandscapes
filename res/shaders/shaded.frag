#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D tex;

void main() {
    vec4 res = texture(tex, uv);

    const vec3 l_dir = normalize(vec3(1.0));
    vec3 norm = res.xyz;

    float dif = clamp(dot(l_dir, norm), 0.0, 1.0); //texture(tex, uv);
    frag_col = vec4(vec3(dif), 1.0);
}