#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D heightmap;

float getHeight(vec2 p) {
    return texture(heightmap, p).r;
}

vec3 getNorm(vec2 p) {
    vec2 h = vec2(0.0, 0.00001);

    return normalize(vec3(
        getHeight(p+h.yx) - getHeight(p-h.yx),
        2.0*h.y,
        getHeight(p+h.xy) - getHeight(p-h.xy)
    ));
}

void main() {
    //float height = getHeight(uv);
    vec3 norm = getNorm(uv);

    frag_col = vec4(0.5*norm+0.5, 1.0);    
}
