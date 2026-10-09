Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

cbuffer ShatterParams : register(b4)
{
    float4 pieceUV; // (uvX, uvY, uvW, uvH)
    float4 transform; // (cx, cy, rot, alpha)
    float4 screenSize;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float2 SubUV : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 col = g_Texture.Sample(g_Sampler, input.TexCoord);

    float2 localUV =
        (input.TexCoord - input.SubUV) / pieceUV.zw;

    float2 edge = abs(localUV * 2.0f - 1.0f);
    float rim = pow(max(edge.x, edge.y), 4.0f);

    float3 crackColor = float3(0.6f, 0.85f, 1.0f);

    col.rgb *= (1.0f - rim * 0.3f);
    col.rgb += crackColor * rim * 0.5f;

    return col;
}