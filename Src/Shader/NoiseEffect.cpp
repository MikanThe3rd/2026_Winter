#include "NoiseEffect.h"
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"

NoiseEffect::NoiseEffect()
{
}

NoiseEffect::~NoiseEffect()
{
}

void NoiseEffect::Init(void)
{
	PostEffectBase::Init();

	// ポストエフェクト用スクリーン
	shader_ = LoadPixelShader(
		(Application::PATH_SHADER + "GlitchPS.cso").c_str());

	constBuf_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * 2);

}

void NoiseEffect::Update(void)
{
    
	elapsedTime_ += 1.0f / 60.0f;
}

void NoiseEffect::SetConstantBuffer()
{
    FLOAT4* glitchBufPtr = (FLOAT4*)GetBufferShaderConstantBuffer(constBuf_);

    // 時間, 変更量, サイズ, ShiftAmount
    glitchBufPtr[0].x = elapsedTime_;     // 経過秒数
    glitchBufPtr[0].y = intensity_;   // Intensity
    glitchBufPtr[0].z = blockSize_;   // BlockSize
    glitchBufPtr[0].w = shiftAmount_; // ShiftAmount

    // FLOAT4[1]: GlitchFreq, ScreenWidth, ScreenHeight, pad
    glitchBufPtr[1].x = glitchFreq_;
    glitchBufPtr[1].y = (float)Application::SCREEN_SIZE_X;
    glitchBufPtr[1].z = (float)Application::SCREEN_SIZE_Y;
    glitchBufPtr[1].w = 0.0f;
}
