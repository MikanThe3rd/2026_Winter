#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_color; // 乗算色 (RGB) + 強度 (A)
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    // UV座標とテクスチャを参照して、元の色を取得する
    float4 srcCol = tex.Sample(texSampler, PSInput.uv);
    if (srcCol.a < 0.01f)
    {
        // 描画しない(アルファテスト)
        discard;
    }

    float4 dstCol = srcCol;

    // モノクロ化
    float gray = dot(srcCol.rgb, float3(0.3f, 0.3f, 0.3f));
    float3 monoCol = float3(gray, gray, gray);

    // 乗算色を適用 (g_color.aで効果の強さを調整可能)
    float3 tintedCol = monoCol * g_color.rgb;
    dstCol.rgb = lerp(monoCol, tintedCol, g_color.a);

    return dstCol;
}