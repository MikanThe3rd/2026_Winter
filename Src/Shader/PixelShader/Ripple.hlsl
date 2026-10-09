#include "../Common/Pixel/PixelShader2DHeader.hlsli"

#define MAX_RIPPLES 4

cbuffer RippleCB : register(b4)
{
    float4 g_Ripples[MAX_RIPPLES];
    float4 g_Param;
    float4 g_Param2;
};

float2 CalcRippleOffset(float2 uv, float4 r)
{
    float2 center = r.xy;
    float time = r.z;

    float speed = g_Param.x;
    float freq = g_Param.y;
    float amplitude = g_Param.z;
    float decay = g_Param.w;
    float maxRadius = g_Param2.x;

    float2 diff = uv - center;
    float dist = length(diff);
    float2 dir = (dist > 0.0001f) ? (diff / dist) : float2(0.0f, 1.0f);

    float waveFront = time * speed;
    float delta = dist - waveFront;

    float distFade = saturate(1.0f - dist * decay * 0.25f);
    float waveFade = saturate(1.0f - abs(delta) * freq * 0.3f); 
    float radiusFade = saturate(1.0f - dist / maxRadius);
    float birthFade = saturate(time * 6.0f);

    float envelope = distFade * waveFade * radiusFade * birthFade;

    float wave = sin((delta * freq - time * 0.5f) * 3.14159f * 2.0f)
               * envelope * amplitude;

    return dir * wave;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;
    float2 offset = float2(0.0f, 0.0f);

    if (g_Ripples[0].w > 0.5f)
        offset += CalcRippleOffset(uv, g_Ripples[0]);
    if (g_Ripples[1].w > 0.5f)
        offset += CalcRippleOffset(uv, g_Ripples[1]);
    if (g_Ripples[2].w > 0.5f)
        offset += CalcRippleOffset(uv, g_Ripples[2]);
    if (g_Ripples[3].w > 0.5f)
        offset += CalcRippleOffset(uv, g_Ripples[3]);

    float2 sampleUV = clamp(uv + offset, 0.0f, 1.0f);
    return tex.Sample(texSampler, sampleUV) * PSInput.diffuse;
}