//PBR - work in progress, based on https://learnopengl.com/PBR/Lighting
//Currently supports dielectric materials only

#define PI 3.1415926535

#define sat(x) clamp(x, 0.0, 1.0)

float D_GGX(vec3 n, vec3 h, float a) {
    float a2     = a*a;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 F_Schlick(vec3 v, vec3 h, vec3 f0) {
    float f = pow(1.0 - sat(dot(v,h)), 5.0);
    return f0 + (1.0-f0)*f;
}

vec3 ShadePBR(vec3 view, vec3 norm, vec3 ldir,
                vec3 albedo, float roughness)
{
    const vec3 reflectance = vec3(0.04);

    //Easier to tweak parametrization
    roughness *= roughness;

    vec3 h = normalize(ldir + view);
    
    float D = D_GGX(norm, h, roughness);
    float G = GeometrySmith(norm, view, ldir, roughness);
    vec3  F = F_Schlick(view, h, reflectance); 
    
    vec3 numerator    = D * G * F;
    float denominator = 4.0 * sat(dot(norm, view)) * sat(dot(norm, ldir)) + 0.0001;
    vec3 specular     = numerator / denominator;  
    
    //Assumed metalic = 0
    vec3 kD = vec3(1.0) - F;
    
    float NoL = sat(dot(norm, ldir));

    return (kD*albedo/PI + specular) * NoL;
}

vec3 IBL(samplerCube irradiance, samplerCube prefiltered, vec3 norm, vec3 view, vec3 albedo, float roughness)
{
    const vec3 F0 = vec3(0.04);

    float cosTheta = max(0.0, dot(norm, view));
    vec3 F = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);

    vec3 kD = 1.0 - F;
    vec3 irr = uSkyDiff * texture(irradiance, norm).rgb;

    //4.0 = log2(res) - 2
    float lod = 4.0*roughness;
    vec3 refl = reflect(-view, norm);
    vec3 pref = uSkySpec * textureLod(prefiltered, refl, lod).rgb;

    return kD * irr * albedo + F * pref;
}

vec3 diffuseOnly(vec3 norm, vec3 ldir, vec3 albedo)
{
    float NoL = sat(dot(norm, ldir));
    return (albedo/PI) * NoL;
}