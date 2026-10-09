#include "RippleEffect.h"
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/InputManager.h"

RippleEffect::RippleEffect()
{
}

RippleEffect::~RippleEffect()
{
}

void RippleEffect::Init(void)
{
    PostEffectBase::Init();

    shader_ = LoadPixelShader(
        (Application::PATH_SHADER + "Ripple.cso").c_str());

    // ロード失敗時にログ出力
    if (shader_ == -1) {
        MessageBox(NULL, "RipplePS.cso の読み込み失敗", "Shader Error", MB_OK);
    }

    constBuf_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * (MAX_RIPPLES + 2));
}

void RippleEffect::Update(void)
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgMouseLeft()) {
        Vector2 mousePos = ins.GetMousePos();
        AddRipple((int)mousePos.x, (int)mousePos.y);
    }
    

    float dt = 1.0f / 60.0f;
    float lifetime = maxRadius_ / speed_ + 0.5f;

    for (auto& r : ripples_)
    {
        if (r.alive < 0.5f) continue;

        r.time += dt;
        if (r.time > lifetime) {
            r.alive = 0.0f;
        }
    }
}

void RippleEffect::AddRipple(int screenX, int screenY)
{
    // 空きスロットを探して追加
    for (auto& r : ripples_)
    {
        if (r.alive < 0.5f)
        {
            r.uvX = static_cast<float>(screenX) / Application::SCREEN_SIZE_X;
            r.uvY = static_cast<float>(screenY) / Application::SCREEN_SIZE_Y;
            r.time = 0.0f;
            r.alive = 1.0f;
            return;
        }
    }

    // 全スロット使用中なら最も古いものを上書き
    RippleData* oldest = &ripples_[0];
    for (auto& r : ripples_) {
        if (r.time > oldest->time) oldest = &r;
    }
    oldest->uvX = static_cast<float>(screenX) / Application::SCREEN_SIZE_X;
    oldest->uvY = static_cast<float>(screenY) / Application::SCREEN_SIZE_Y;
    oldest->time = 0.0f;
    oldest->alive = 1.0f;
}

void RippleEffect::SetConstantBuffer()
{
    FLOAT4* buf = (FLOAT4*)GetBufferShaderConstantBuffer(constBuf_);

    // FLOAT4[0?7] : 波紋データ (uvX, uvY, time, alive)
    for (int i = 0; i < MAX_RIPPLES; i++)
    {
        buf[i].x = ripples_[i].uvX;
        buf[i].y = ripples_[i].uvY;
        buf[i].z = ripples_[i].time;
        buf[i].w = ripples_[i].alive;
    }

    // FLOAT4[8] : speed, freq, amplitude, decay
    buf[MAX_RIPPLES].x = speed_;
    buf[MAX_RIPPLES].y = freq_;
    buf[MAX_RIPPLES].z = amplitude_;
    buf[MAX_RIPPLES].w = decay_;

    // FLOAT4[9] : maxRadius, pad, pad, pad
    buf[MAX_RIPPLES + 1].x = maxRadius_;
    buf[MAX_RIPPLES + 1].y = 0.0f;
    buf[MAX_RIPPLES + 1].z = 0.0f;
    buf[MAX_RIPPLES + 1].w = 0.0f;
}