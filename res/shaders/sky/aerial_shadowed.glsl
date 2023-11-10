#version 450 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(rgba16, binding = 0) uniform image3D aerialLUT;

uniform sampler3D scatterVolume;
uniform sampler3D shadowVolume;

uniform float uBrightness;

uniform int uShadows;

void main() {
    ivec3 texelCoord = ivec3(gl_GlobalInvocationID.xyz);

    //Raymarch pre-generated volumes
    float transmittance = 1.0;
    vec3 in_scatter = vec3(0.0);

    for (int i=0; i<=texelCoord.z; i++)
    {
        ivec3 current_index = ivec3(texelCoord.xy, i);

        //Normalized 3d coordinates [0,1]
        vec3 coord = (vec3(current_index)+0.5)/imageSize(aerialLUT);

        vec4 scatter = texture(scatterVolume, coord);

        vec3 IntS = scatter.rgb; 
        float mean_sample_trans = scatter.a;

        if (uShadows == 1)
        {
            //Note: shadowing here is not correct as it also
            //affects the multicasttering term, but avoiding thus
            //would require generating yet another volume
            float shadow = texture(shadowVolume, coord).r;
            IntS *= shadow;
        }

        //Usual integration
        in_scatter += transmittance*IntS;
        transmittance *= mean_sample_trans;
    }

    //Save
    vec4 res = vec4(uBrightness*in_scatter, transmittance);

    //Debug shadows
    //Normalized 3d coordinates [0,1]
    vec3 coord = (vec3(texelCoord)+0.5)/imageSize(aerialLUT);
    float shadow = texture(shadowVolume, coord).r;

    //res = vec4(vec3(shadow), 1.0);
    //res = vec4(1,1,0,0);

    imageStore(aerialLUT, texelCoord, res);
}