#pragma once
#include "PostEffectBase.h"

class MonotoneEffect : public PostEffectBase {
public:

    void Init() override;

protected:
    void SetConstantBuffer() override;
};

