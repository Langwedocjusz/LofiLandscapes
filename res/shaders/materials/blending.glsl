#define BLEND_AVERAGE     0
#define BLEND_ADD         1
#define BLEND_SUBTRACT    2
#define BLEND_AVG_W_CONST 3

uniform int uBlendMode;
uniform float uWeight;
uniform float uSmoothClamp;

//Based on exponential variant from article by Inigo Quilez:
//https://iquilezles.org/articles/smin/
float smin(float a, float b, float k)
{
    if (k <= 0.1)
        return min(a,b);

    //k *= 1.0;
    k = pow(k, 2.0);
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
}

float smax(float a, float b, float k)
{
    return -smin(-a, -b, k);
}

float BlendResult(float prev, float current)
{
    float res = 0.0;

    //float h = clamp(current, 0.0, 1.0);
    float h = smax(0.0, current, uSmoothClamp);
    h = smin(1.0, h, uSmoothClamp);

    switch(uBlendMode) {
        case BLEND_AVERAGE:
        {
            res = mix(prev, h, uWeight);
            break;
        }
        case BLEND_ADD:
        {
            res = prev + uWeight*h;
            break;
        }
        case BLEND_SUBTRACT:
        {
            res = prev - uWeight*h;
            break;
        }
        case BLEND_AVG_W_CONST:
        {
            res = mix(prev, uWeight, h);
            break;
        }
    }

    return clamp(res, 0.0, 1.0);
}