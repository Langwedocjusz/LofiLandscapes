#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image3D raycast_result;

#define PI 3.1415926536

uniform float uViewAngle;

uniform float uBaseWidth;
uniform float uSlant;

float sdEllipse(vec2 p, vec2 ab);
float opSubtract(float d1, float d2);
vec2 opRepeat(vec2 p, vec2 period, out vec2 outId);
vec4 hash42(vec2 p);
mat2 rotate2d(float theta);

//Grass generation based on the amazing shader
//"grass field with blades" by MonterMan:
//https://www.shadertoy.com/view/dd2cWh

//My modification/generalization of his grass blade sdf is also available here;
//https://www.shadertoy.com/view/DdlfR7

float sdGrassBlade2d(vec2 p, float slant, float base_width)
{   
    float s = sin(PI-slant), c = cos(PI-slant);
    
    //Factor scaling verical axes of the ellipses
    //to make y coord of the intersection always equal to 1.0
    float vert_fac = 1.0/s;

    //Initial, larger ellipse 
    //It is placed at the lower right cornet, with horizontal axis = 1.0
    float dist = sdEllipse(p - vec2(1.0, 0.0), vec2(1.0, vert_fac));
    
    //Computing minor axis of the smaller ellipse such that it intersects
    //the larger one at the given angle (slant), but keeping width of the
    //base fixed
    float C = 1.0 - base_width;
    
    float r = (1.0 + C*C + 2.0*C*c)/(2.0 + 2.0*C*c);
    
    //Constructing the smaller ellipse
    vec2 axes = vec2(r, r*vert_fac);
    
    vec2 offset = (1.0 - r) * vec2(c, vert_fac * s);
    vec2 center = vec2(1.0, 0.0) + offset;
    
    //Carving out the smaller ellipse
    dist = opSubtract(dist, sdEllipse(p - center, axes));
    
    //Removing everything below the y=0.0 plane
    dist = opSubtract(dist, p.y);
    
    //Removing everything above the tip using
    //sdf of a plane that is touched by the tip at a right angle
    vec2 org = vec2(1.0, 0.0);
    vec2 dir = normalize(vec2(-1.0, c));
    
    dist = opSubtract(dist, dot(p-org, dir));
    
    return dist;
}

float sdGrassBlade(vec3 p, float thickness)
{   
    float dist2d = max(0.0, sdGrassBlade2d(p.xy, uSlant, uBaseWidth));
    
    return sqrt(dist2d*dist2d + p.z*p.z) - thickness;
}

float Map(vec3 p)
{
    vec2 grassId;
    float repeatPeriod = 0.25;
    p.xz = opRepeat(p.xz, vec2(repeatPeriod), grassId);
    
    float dist = 100.0;
    
    #define NEIGHBORS_TO_CHECK 2

    for (int dy = -NEIGHBORS_TO_CHECK; dy <= NEIGHBORS_TO_CHECK; ++dy)
    {
        for (int dx = -NEIGHBORS_TO_CHECK; dx <= NEIGHBORS_TO_CHECK; ++dx)
        {
            vec3 neighborP = p - vec3(dx, 0, dy) * repeatPeriod;
            vec2 neighborId = grassId + vec2(dx, dy);
            
            vec4 rand = hash42(neighborId);
            neighborP.xz *= rotate2d(rand.z*6.28);
            neighborP.xz += (rand.xy - 0.5) * repeatPeriod;
            
            dist = min(dist, sdGrassBlade(neighborP/sqrt(rand.w), 0.002));
        }
    }

    return dist;
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

bool Raymarch(vec3 org, vec3 dir, inout float dist) {
    const int max_march_steps = 512;
    const float min_dist = 0.001, max_dist = 100.0;

    dist = 0.0;

    if (Map(org) < 0.0) return true;

    for (int i=0; i<max_march_steps; i++)
    {   
        vec3 p = org + dist*dir;

        if (p.y < 0.0) break;

        float sd = Map(p);
        if (abs(sd) < min_dist) return true;

        dist += sd;
        if (dist > max_dist) break;
    }

    return false;
}


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

    //Multisample (uniform grid)

    float texel_fourth = 0.25/float(img_size.x); // Assuming xy has square dims

    vec2 offsets[16] = vec2[16](
        vec2(-1.5, 1.5), vec2(-0.5, 1.5), vec2(0.5, 1.5), vec2(1.5, 1.5),
        vec2(-1.5, 0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5), vec2(1.5, 0.5),
        vec2(-1.5,-0.5), vec2(-0.5,-0.5), vec2(0.5,-0.5), vec2(1.5,-0.5),
        vec2(-1.5,-1.5), vec2(-0.5,-1.5), vec2(0.5,-1.5), vec2(1.5,-1.5)
    );

    float dist = 0.0, normalization = 0.0;

    for (int i = 0; i<16; i++)
    {
        vec2 org2 = uv + texel_fourth * offsets[i];
        vec3 org3 = vec3(org2.x, 1.0, org2.y);

        bool hit = Raymarch(org3, dir, dist);

        if (hit)
        {
            res.rgb += Norm(org3 + dist*dir);
            res.a   += dist;

            normalization += 1.0;
        }
        
    }

    if (normalization > 0.0)
        res /= normalization;

    else res.a = 10.0;

    imageStore(raycast_result, texelCoord, res);
}

// hash function credit: https://www.shadertoy.com/view/4djSRW
vec4 hash42(vec2 p)
{
	vec4 p4 = fract(vec4(p.xyxy) * vec4(.1031, .1030, .0973, .1099));
    p4 += dot(p4, p4.wzxy+33.33);
    return fract((p4.xxyz+p4.yzzw)*p4.zywx);
}

mat2 rotate2d(float theta)
{
    float c = cos(theta);
    float s = sin(theta);
    return mat2(c, -s,s, c);
}

float opSubtract(float d1, float d2) 
{ 
    return max(d1,-d2);
}

vec2 opRepeat(vec2 p, vec2 period, out vec2 outId)
{
    outId = floor((p+0.5*period)/period);
    return mod(p+0.5*period, period) - 0.5*period;
}

//Ellipse SDF by Inigo Quilez: https://www.shadertoy.com/view/4lsXDN
float sdEllipse( vec2 p, vec2 ab )
{
    // symmetry
	p = abs( p );

    // find root with Newton solver
    vec2 q = ab*(p-ab);
	float w = (q.x<q.y)? 1.570796327 : 0.0;
    for( int i=0; i<5; i++ )
    {
        vec2 cs = vec2(cos(w),sin(w));
        vec2 u = ab*vec2( cs.x,cs.y);
        vec2 v = ab*vec2(-cs.y,cs.x);
        w = w + dot(p-u,v)/(dot(p-u,u)+dot(v,v));
    }
    
    // compute final point and distance
    float d = length(p-ab*vec2(cos(w),sin(w)));
    
    // return signed distance
    return (dot(p/ab,p/ab)>1.0) ? d : -d;
}