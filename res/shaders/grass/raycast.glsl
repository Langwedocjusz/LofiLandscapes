#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image3D raycast_result;

#define PI 3.1415926536

uniform float uTaper;

float sdfSphere(vec2 p, float r) {
    return length(p) - r;
}

float Map(vec2 p) {
    vec2 q = fract(p);
    return sdfSphere(q - vec2(0.5), 0.25);
}

vec3 Norm(vec2 p)
{
    if (Map(p) < 0.0) return vec3(0.0, 1.0, 0.0);

    vec2 n = normalize(fract(p) - vec2(0.5));

    return vec3(n.x, 0.0, n.y);
}

float Raymarch(vec2 org, vec2 dir) {
    const int max_march_steps = 64;
    const float min_dist = 0.01, max_dist = 100.0;

    float dist = 0.0;

    if (Map(org) < 0.0) return dist;

    for (int i=0; i<max_march_steps; i++)
    {   
        vec2 p = org + dist*dir;

        float sd = Map(p);
        if (abs(sd) < min_dist) break;

        dist += sd;
        if (dist > max_dist) break;
    }

    return dist;
}


#define MULTISAMPLE

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    ivec3 img_size = imageSize(raycast_result);

    vec2 uv = (vec2(texelCoord.xy) + 0.5)/ img_size.xy;

    float angle = 2.0*PI*(float(texelCoord.z) + 0.5)/img_size.z;

    vec2 dir = vec2(cos(angle), sin(angle));

    vec4 res = vec4(0.0);

    #ifdef MULTISAMPLE
    float texel_third = 0.33/float(img_size.x); // Assuming xy has square dims

    vec2 offsets[9] = vec2[9](
        vec2(-1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 1.0),
        vec2(-1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 0.0),
        vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0)
    );

    for (int i = 0; i<9; i++)
    {
        vec2 p = uv + texel_third * offsets[i];

        float dist = Raymarch(p, dir);

        res.rgb += Norm(p + dist*dir);
        res.a   += dist;
    }

    res /= 9.0;
    
    #else
    float dist = Raymarch(uv, dir);

    res.rgb = Norm(uv + dist*dir);
    res.a   = dist;
    #endif

    imageStore(raycast_result, texelCoord, res);
}