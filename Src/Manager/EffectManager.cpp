#include "EffectManager.h"
#include "../Application.h"
#include "../Shader/MonotoneEffect.h"
#include"../Shader/NoiseEffect.h"
#include"../Shader/RippleEffect.h"
#include"../Shader/ShatterEffect.h"
#include"InputManager.h"

void EffectManager::Init()
{
    pingPong_[0] = MakeScreen(
        Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, true);
    pingPong_[1] = MakeScreen(
        Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, true);

    Add<MonotoneEffect>();
    Add<NoiseEffect>();

    rippleEffect_ = Add<RippleEffect>();
    shatterEffect_ = Add<ShatterEffect>();

}

void EffectManager::Cleanup()
{
    DeleteGraph(pingPong_[0]);
    DeleteGraph(pingPong_[1]);
}

void EffectManager::Update()
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgDown(KEY_INPUT_M)) {
        ToggleEffect<MonotoneEffect>();
    }

    if (ins.IsTrgDown(KEY_INPUT_N)) {
        ToggleEffect<NoiseEffect>();
    }

    if (ins.IsTrgDown(KEY_INPUT_R)) {
        ToggleEffect<RippleEffect>();
    }

    if (ins.IsTrgDown(KEY_INPUT_B)) {
        ToggleEffect<ShatterEffect>();
    }
    
    for (auto& e : effects_) {
        e->UpdateIfEnabled();
    }
}

void EffectManager::Draw(int mainScreen)
{
    // 有効なエフェクトの数を数える
    int enabledCount = 0;
    for (auto& e : effects_) {
        if (e->IsEnabled()) enabledCount++;
    }

    // 有効なエフェクトが無ければ何もしない（mainScreenはそのまま）
    if (enabledCount == 0) {
        return;
    }

    // mainScreenの内容をpingPong_[0]にコピー
    SetDrawScreen(pingPong_[0]);
    DrawGraph(0, 0, mainScreen, false);

    int cur = 0;
    int processed = 0;
    for (size_t i = 0; i < effects_.size(); i++) {
        if (!effects_[i]->IsEnabled()) continue;

        processed++;
        int next = 1 - cur;
        int dst = (processed == enabledCount) ? mainScreen : pingPong_[next];
        effects_[i]->Apply(pingPong_[cur], dst);
        cur = next;
    }
}