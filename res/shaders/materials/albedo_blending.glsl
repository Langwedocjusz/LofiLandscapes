#define BLEND_AVERAGE  0
#define BLEND_ADD      1
#define BLEND_SUBTRACT 2
#define BLEND_MULTIPLY 3

uniform int uBlendMode;
uniform float uWeight;

vec3 BlendResult(vec3 prev, vec3 current)
{
    vec3 res = vec3(0.0);

    switch(uBlendMode) {
        case BLEND_AVERAGE:
        {
            res = mix(prev, current, uWeight);
            break;
        }
        case BLEND_ADD:
        {
            res = prev + uWeight*current;
            break;
        }
        case BLEND_SUBTRACT:
        {
            res = prev - uWeight*current;
            break;
        }
        case BLEND_MULTIPLY:
        {
            res = mix(prev, current*prev, uWeight);
            break;
        }
    }

    return clamp(res, 0.0, 1.0);
}