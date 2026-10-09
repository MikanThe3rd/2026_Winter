#pragma once
#include "PostEffectBase.h"


class RippleEffect :
    public PostEffectBase
{
public:
    RippleEffect();
    ~RippleEffect();

    void Init(void) override;
    void Update(void) override;

    // マウスクリック位置に波紋を追加（スクリーン座標で渡す）
    void AddRipple(int screenX, int screenY);

protected:
    void SetConstantBuffer() override;

private:
    // 同時に扱える最大波紋数（シェーダー側と合わせること）
    static constexpr int MAX_RIPPLES = 4;

    // 波紋1個の状態
    struct RippleData
    {
        float uvX = 0.0f;  // クリック位置 U (0-1)
        float uvY = 0.0f;  // クリック位置 V (0-1)
        float time = 0.0f;  // 発生からの経過時間 (秒)
        float alive = 0.0f;  // 1.0f=有効, 0.0f=無効
    };

    RippleData ripples_[MAX_RIPPLES] = {};

    float speed_ = 0.4f;    // 速く広げる
    float freq_ = 6.0f;   // 波を少し粗くして輪っかを強調
    float amplitude_ = 1.0f;  // 少し強め
    float decay_ = 1.0f;    // 遠くまで届くように緩める
    float maxRadius_ = 1.5f;
};
