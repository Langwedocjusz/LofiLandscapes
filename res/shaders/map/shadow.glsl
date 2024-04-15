#version 450 core

#define SOFT_SHADOWS

#define PI 3.1415926535

#define saturate(x) clamp(x, 0.0, 1.0)

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(r8, binding = 0) uniform image2D shadowmap;

uniform int uResolution;

uniform sampler2D heightmap;

uniform float uScaleXZ;
uniform float uScaleY;
uniform vec3 uSunDir;

uniform int uMipOffset;
uniform int uMinLvl;
uniform int uStartCell;
uniform float uNudgeFactor;

uniform bool uSoftShadows;
uniform float uSharpness;

const float epsilon = 2*1e-6;

float DistToNextIntersection(vec2 pos, vec2 dir2, vec2 delta, float cell_size)
{
    float tx = dir2.x < 0.0
             ? (pos.x/cell_size - floor(pos.x/cell_size))*cell_size * delta.x
             : (ceil(pos.x/cell_size) - pos.x/cell_size) *cell_size * delta.x;

    float ty = dir2.y < 0.0
             ? (pos.y/cell_size - floor(pos.y/cell_size))*cell_size * delta.y
             : (ceil(pos.y/cell_size) - pos.y/cell_size) *cell_size * delta.y;
                  
    return min(tx, ty);
}

bool WithinTexture(vec2 pos)
{
    return (pos.x > uResolution*epsilon && pos.x < uResolution*(1.0-epsilon) 
         && pos.y > uResolution*epsilon && pos.y < uResolution*(1.0-epsilon));
}

bool IntersectTerrain(vec2 org2, vec2 dir2, vec3 org3, vec3 dir3, 
                      float t_start, float t_delta, float proj_fac,
                      inout float mindh, inout float mint)
{
    const int samples = 5;
    const float ve = 1.0/float(samples-1);

    float texel_size = 1.0/uResolution;
    
    for (int i=0; i<5; i++) {
        float t = t_start + ve*float(i)*t_delta;

        vec2 p2 = org2 + t*dir2;
        vec3 p3 = org3 + proj_fac*t*dir3;

        float h = texture(heightmap, texel_size*p2).r;
        float H = texel_size*p3.y;

        float dh = H-h;

        if (uSoftShadows && dh < mindh) {
            mindh = dh;
            mint = t;
        }

        if (!uSoftShadows && dh < 0.0)
            return true; 
    }

    return false;
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    //Not normalized (from 0 to uResolution)
    vec2 uv = vec2(texelCoord);
    
    //Get sun ray origin and direction:
    float texel_size = 1.0/uResolution;
    
    float height = texture(heightmap, texel_size*uv).r;

    vec3 org3 = vec3(uv.x, uResolution*height, uv.y);

    vec3 dir3 = normalize(vec3(uScaleY  * uSunDir.x,
                               uScaleXZ * uSunDir.y,
                               uScaleY  * uSunDir.z));

    //This will be useful to project back to 3d
    float proj_fac = 1.0/(dot(dir3, vec3(1.0, 0.0, 1.0)*dir3));

    //2d values
    vec2 org2 = org3.xz;
    vec2 dir2 = normalize(dir3.xz);

    vec2 pos = org2 + 0.01*dir2;

    //Constants derived from ray direction 
    vec2 delta;
         delta.x  = abs(dir2.x) < texel_size ? 1e30 : abs(1.0/dir2.x);
         delta.y  = abs(dir2.y) < texel_size ? 1e30 : abs(1.0/dir2.y);

    int cell_size = uStartCell;
    int mip = uMinLvl;

    float min_dh = 1.0, min_t = 1.0;

    float t = DistToNextIntersection(pos, dir2, delta, float(cell_size));

    bool ray_above_terrain = !IntersectTerrain(org2, dir2, org3, dir3, 0.0, t, proj_fac, min_dh, min_t);

    for (int i=0; i<36; i++)
    {
        if (!ray_above_terrain) break;

        pos = org2 + uNudgeFactor*t*dir2;
        
        if (!WithinTexture(pos)) break;

        ivec2 current_texel = ivec2(pos);
        current_texel = current_texel / cell_size;

        //Calculate next grid intersection
        float tmp_t = DistToNextIntersection(pos, dir2, delta, float(cell_size));

        //Check for terrain intersection
        if (mip == uMinLvl)
        {
            if (IntersectTerrain(org2, dir2, org3, dir3, t, tmp_t, proj_fac, min_dh, min_t))
            {
                ray_above_terrain = false;
                continue;
            }
        }

        else
        {
            float h = texelFetch(heightmap, current_texel, mip + uMipOffset).r;
            
            vec3 tmp = texel_size*(org3 + proj_fac*t*dir3);
            float H = tmp.y;

            float dh = H-h;

            if (dh < epsilon)
            {
                cell_size /= 2;
                mip--;

                continue;
            }
        }

        //Move to next intersection
        t += tmp_t;

        //If we are passing through a border of a larger scale grid cell try to increase mip
        if ( int(pos.x) % 2*int(cell_size + 1e-5) == 0
          || int(pos.y) % 2*int(cell_size + 1e-5) == 0);
        {
            cell_size *= 2;
            mip++;
        }
    }

    float shadow = 1.0;

    if (uSoftShadows)
        shadow = 1.0 - saturate(-uSharpness*min_dh/max(texel_size*min_t, 0.01));
    else
        shadow = float(ray_above_terrain);

    imageStore(shadowmap, texelCoord, vec4(shadow, 0.0, 0.0, 1.0));
}
