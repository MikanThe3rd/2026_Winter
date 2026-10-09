#include "MonotoneEffect.h"
#include "../Application.h"

void MonotoneEffect::Init()
{
    PostEffectBase::Init();

    shader_ = LoadPixelShader(
        (Application::PATH_SHADER + "Monotone.cso").c_str());

    constBuf_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * 1);

    
}

void MonotoneEffect::SetConstantBuffer()
{
    // 定数バッファ
    FLOAT4* constBufsPtr =
        (FLOAT4*)GetBufferShaderConstantBuffer(constBuf_);
    // 乗算色
    constBufsPtr->x = 1.0f;
    constBufsPtr->y = 1.0f;
    constBufsPtr->z = 1.0f;
    constBufsPtr->w = 1.0f;
}
