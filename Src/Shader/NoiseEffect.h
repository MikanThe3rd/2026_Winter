#pragma once
#include "PostEffectBase.h"

class NoiseEffect :
    public PostEffectBase
{
public:
    NoiseEffect();
    ~NoiseEffect();

    void Init(void)override;
    void Update(void)override;

protected:

    void SetConstantBuffer() override;

private:
    float elapsedTime_ = 0.0f;
    float intensity_ = 0.5f;
    float blockSize_ = 16.0f;
    float shiftAmount_ = 0.05f;
    float glitchFreq_ = 1.5f;
};