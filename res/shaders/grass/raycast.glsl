#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image3D raycast_result;

#define PI 3.1415926536

uniform float uConeAngle;
uniform float uViewAngle;

float sdfSphere(vec2 p, float r) {
    return length(p) - r;
}

//https://iquilezles.org/articles/distfunctions/
float sdfCone( vec3 p, vec2 c )
{
    // c is the sin/cos of the angle
    vec2 q = vec2( length(p.xz), -p.y );
    float d = length(q-c*max(dot(q,c), 0.0));
    return d * ((q.x*c.y-q.y*c.x<0.0)?-1.0:1.0);
}

float Map(vec3 p) {
    vec3 q = p;
    q.xz = fract(q.xz);

    //return sdfSphere(q.xz - vec2(0.5), 0.25);
    vec2 c = vec2(sin(uConeAngle), cos(uConeAngle));
    return sdfCone(q - vec3(0.5, 0.0, 0.5), c);
}

vec3 Norm(vec3 p)
{
    vec2 h = vec2(0.0, 0.01);

    return normalize(vec3(
        Map(p + h.yxx) - Map(p - h.yxx),
        Map(p + h.xyx) - Map(p - h.xyx),
        Map(p + h.xxy) - Map(p - h.xxy)
    ));
}

float Raymarch(vec3 org, vec3 dir) {
    const int max_march_steps = 64;
    const float min_dist = 0.01, max_dist = 100.0;

    float dist = 0.0;

    if (Map(org) < 0.0) return dist;

    for (int i=0; i<max_march_steps; i++)
    {   
        vec3 p = org + dist*dir;

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

    float c_v = cos(uViewAngle), s_v = sin(uViewAngle);

    float angle = -2.0*PI*(float(texelCoord.z) + 0.5)/img_size.z;
    float c = cos(angle), s = sin(angle);

    //vec2 dir = vec2(cos(angle), sin(angle));
    vec3 dir = vec3(1,0,0);
    dir.xy = mat2(c_v, -c_v, c_v, c_v) * dir.xy;
    dir.xz = mat2(c, -s, s, c) * dir.xz;

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
        vec2 org2 = uv + texel_third * offsets[i];
        vec3 org3 = vec3(org2.x, 0.0, org2.y);

        float dist = Raymarch(org3, dir);

        res.rgb += Norm(org3 + dist*dir);
        res.a   += dist;
    }

    res /= 9.0;
    
    #else

    vec3 org = vec3(uv.x, 0.0, uv.y);

    float dist = Raymarch(org, dir);

    res.rgb = Norm(org + dist*dir);
    res.a   = dist;

    #endif

    imageStore(raycast_result, texelCoord, res);
}