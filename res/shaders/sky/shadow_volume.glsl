#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r16f, binding = 1) uniform image3D shadowVolume;

uniform sampler2D shadowmap;

uniform vec3 uPos;
uniform vec3 uSunDir;

uniform float uFar;
uniform float uNear;

uniform vec3 uFront;

uniform vec3 uBotLeft;
uniform vec3 uBotRight;
uniform vec3 uTopLeft;
uniform vec3 uTopRight;

uniform float uL;
uniform float uScaleY;
uniform float uScaleXZ;

vec2 getShadowUV(vec3 pos)
{
    //Intersect with ground along the sun direction    
    vec3 sun_dir = normalize(vec3(uScaleY, uScaleXZ, uScaleY)*uSunDir);

    float t_hit = - pos.y/sun_dir.y;
    vec3 hit_point = pos + t_hit*sun_dir;
    
    //Translate to uv coordinates
    vec2 shadow_uv = (uL/2.0)*hit_point.xz/uScaleXZ;
    shadow_uv = 0.5*shadow_uv + 0.5;

    return shadow_uv;
}

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    //Normalized 3d coordinates [0,1]
    vec3 coord = (vec3(texelCoord)+0.5)/imageSize(shadowVolume);

    //Retrieve ray direction
    vec3 dir = mix(mix(uBotLeft, uBotRight, coord.x), mix(uTopLeft, uTopRight, coord.x), coord.y);
    dir = normalize(dir);

    //Calculate world position
    float proj = 1.0/dot(uFront, dir);

    float t0    = proj*uNear;
    float t_end = proj*uFar;

    float tf = t0 + coord.z*(t_end-t0);

    vec3 pos = uPos + tf*dir;

    //Sample shadow map
    #define MULTISAMPLE

    #ifdef MULTISAMPLE
        float dt = (t_end-t0)/imageSize(shadowVolume).z;

        float shadow = 0.0;

        for (int i=-1; i<=1; i++)
        {
            vec2 shadow_uv = getShadowUV(pos + float(i)*0.33*dt*dir);
            //shadow += 0.33*texture(shadowmap, shadow_uv).r;

            shadow += 0.33*textureLod(shadowmap, shadow_uv, 4).r;
        }

    #else
        vec2 shadow_uv = getShadowUV(pos);
        float shadow = texture(shadowmap, shadow_uv).r;
    #endif

    //Save
    vec4 res = vec4(shadow, 0.0, 0.0, 0.0);

    //Debug shadowmap visualization
    //float t_meters = 1000000*tf/(uDistScale);
    //vec3 pos = vec3(1,1,1)*uPos + t_meters*dir;
    //vec2 shadow_uv = (4.0/2.0)*vec2(pos.x, pos.z)/400.0;
    //shadow_uv = 0.5*shadow_uv + 0.5;
    //float shadow = texture(shadowmap, shadow_uv).r;
    //res = vec4(vec3(shadow), 1.0);

    imageStore(shadowVolume, texelCoord, res);
}