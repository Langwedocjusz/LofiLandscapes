#version 450 core

#define saturate(x) clamp(x, 0.0, 1.0)

in vec2 uv;

out vec4 frag_col;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
uniform float uTheta;
uniform float uPhi;

uniform int uSteps;
uniform float uMinT;
uniform float uMaxT;
uniform float uBias;

float getHeight(vec2 p, float lvl) {
    return textureLod(heightmap, p, lvl).r;
}

void main() { 
    //Generate ray:
    float dt = uMaxT/float(uSteps);
    float voffset = dt;

    vec3 org = vec3(uv.x, getHeight(uv, 0.0) + voffset, uv.y);

    vec3 dir = normalize(vec3(cos(uPhi)*sin(uTheta),
                     (uScaleXZ/uScaleY)*cos(uTheta),
                              sin(uPhi)*sin(uTheta)));
    dir *= vec3(-1.0, 1.0, -1.0);

    vec2 perp = mat2(0.0, 1.0, -1.0, 0.0) * normalize(dir.xz);

    //Raymarch 
    float t = 0.0;
    float shadow = 1.0;

    for (int i=0; i<uSteps; i++) {
        vec3 p = org + t*dir;
        float h = p.y - getHeight(p.xz, uBias*t);
        float blur = 1.0/t;
        shadow = min(shadow, 1.0 - saturate(-blur*h));
        t += dt;
    }
        
    //Output:
    frag_col = vec4(shadow, 0.0, 0.0, 1.0);
}
