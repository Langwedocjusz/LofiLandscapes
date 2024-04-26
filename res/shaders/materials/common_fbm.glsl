#include "../common/hash.glsl"

float noise(vec2 p, float scale) {
    vec2 id = floor(p);
    vec2 u = fract(p);

    vec2 va = 2.0*hash22(id+vec2(0,0), scale) - 1.0;
    vec2 vb = 2.0*hash22(id+vec2(1,0), scale) - 1.0;
    vec2 vc = 2.0*hash22(id+vec2(0,1), scale) - 1.0;
    vec2 vd = 2.0*hash22(id+vec2(1,1), scale) - 1.0;

    float a = dot(va, u - vec2(0,0)) + 0.5;
    float b = dot(vb, u - vec2(1,0)) + 0.5;
    float c = dot(vc, u - vec2(0,1)) + 0.5;
    float d = dot(vd, u - vec2(1,1)) + 0.5;

    u = u*u*(3.0-2.0*u);

    float k0 = a;
    float k1 = b-a;
    float k2 = c-a;
    float k3 = a-b-c+d;

    return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
}

float fbm(vec2 p, int octaves, int scale, float roughness) 
{
    float normalization = 0.0;
    float res = 0.0;

    float A = 1.0, a = 1.0;

    for (int i=0; i<octaves; i++) {
        res += A*noise(a*p, float(scale)*a);
        normalization += A;

        a *= 2.0;
        A *= roughness;
    }

    return res/normalization;
}