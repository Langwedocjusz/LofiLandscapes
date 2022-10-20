#version 450 core

in vec2 uv;

out vec4 frag_col;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
uniform float uTheta;
uniform float uPhi;

float getHeight(vec2 p) {
    return texture(heightmap, p).r;
}

void main() {
    const int shadow_steps = 32;
    const float mint = 0.001;
    const float voffset = 5.0 * mint;
    
    //Generate ray:
    vec3 org = vec3(uv.x, getHeight(uv) + voffset, uv.y);

    vec3 dir = normalize(vec3(cos(uPhi)*sin(uTheta),
                     (uScaleXZ/uScaleY)*cos(uTheta),
                              sin(uPhi)*sin(uTheta)));

    dir *= vec3(-1.0, 1.0, -1.0);

    //Raymarch:
    float t = mint;
    float shadow = 1.0;

    for (int i=0; i<shadow_steps; i++) {
        vec3 p = org + t*dir;
        float h = p.y - abs(getHeight(p.xz));
        
        if (h<mint) {
            shadow = 0.0;
            break;
        }

        if (p.x < 0.0 || p.x > 1.0  || p.z < 0.0 || p.z > 1.0) break;
        
        shadow = min(shadow, 2.0*h/t);
        t += h;
    }

    //Output:
    frag_col = vec4(shadow, 0.0, 0.0, 1.0);
}
