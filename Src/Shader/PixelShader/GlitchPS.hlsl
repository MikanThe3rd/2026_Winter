#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer GlitchParams : register(b4)
{
    float4 g_Param0; // x=Time, y=Intensity, z=BlockSize, w=ShiftAmount
    float4 g_Param1; // x=GlitchFreq, y=ScreenWidth, z=ScreenHeight, w=pad
};

#define g_Time        g_Param0.x
#define g_Intensity   g_Param0.y
#define g_BlockSize   g_Param0.z
#define g_ShiftAmount g_Param0.w
#define g_GlitchFreq  g_Param1.x
#define g_ScreenSize  g_Param1.yz

float rand2(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float rand1(float v)
{
    return frac(sin(v * 74.1327) * 43758.5453);
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    float t = floor(g_Time * g_GlitchFreq * 4.0) / 4.0;

    float2 blockUV = floor(uv * g_ScreenSize / g_BlockSize)
                   * g_BlockSize / g_ScreenSize;

    float blockRand = rand2(blockUV + t);
    float glitchOn = step(1.0 - g_Intensity, blockRand);

    float shiftX = (rand1(blockUV.y + t * 0.37) * 2.0 - 1.0)
                 * g_ShiftAmount * glitchOn;

    float rowT = floor(g_Time * g_GlitchFreq * 2.0) / 2.0;
    float rowIdx = floor(uv.y * g_ScreenSize.y / g_BlockSize);
    float rowRand = rand2(float2(rowIdx, rowT));
    float rowGlitch = step(1.0 - g_Intensity * 0.6, rowRand);
    float shiftRow = (rand1(rowIdx + rowT) * 2.0 - 1.0)
                    * g_ShiftAmount * rowGlitch;

    float2 shiftedUV = float2(saturate(uv.x + shiftX + shiftRow), uv.y);

    float4 srcCol = tex.Sample(texSampler, uv);
    if (srcCol.a < 0.01f)
        discard;

    float4 base = tex.Sample(texSampler, shiftedUV);

    float rShift = g_ShiftAmount * 0.5 * glitchOn;
    float r = lerp(base.r, tex.Sample(texSampler, float2(saturate(shiftedUV.x + rShift), shiftedUV.y)).r, glitchOn);
    float g = base.g;
    float b = lerp(base.b, tex.Sample(texSampler, float2(saturate(shiftedUV.x - rShift), shiftedUV.y)).b, glitchOn);

    float replaceRand = rand2(blockUV + t * 1.7 + 3.14);
    float doReplace = step(1.0 - g_Intensity * 0.4, blockRand)
                      * step(0.7, replaceRand);

    r = lerp(r, rand2(blockUV + t * 2.3 + float2(0, 0)), doReplace);
    g = lerp(g, rand2(blockUV + t * 2.3 + float2(1, 0)), doReplace);
    b = lerp(b, rand2(blockUV + t * 2.3 + float2(2, 0)), doReplace);

    return float4(r, g, b, srcCol.a);
}