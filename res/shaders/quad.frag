#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform int uOctaves;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)-0.5;
}

float noise(vec2 p) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    float a = hash(id+vec2(0,0));
    float b = hash(id+vec2(1,0));
    float c = hash(id+vec2(0,1));
    float d = hash(id+vec2(1,1));

    u = u*u*u*(u*(6.0*u-15.0)+10.0);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
}

float fbm(in vec2 p, int octaves) {
    const float scale_y = 1.0;
    const float scale_xz = 0.5;
    const mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);

    p *= scale_xz;

    float res = 0.0;
    mat2 M = mat2(1.0);

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*M*p);

        a *= 2.0;
        A *= 0.5;
        M *= rot;
    }

    return scale_y * res;
}

float getHeight(vec2 p) {
    return fbm(p, uOctaves);
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
    float height = getHeight(uv);
    vec3 norm = getNorm(uv);

    frag_col = vec4(norm, height);    
}
