#include "ShatterEffect.h"
#include "../Application.h"
#include "../Manager/InputManager.h"
#include <cmath>
#include <random>
#include <algorithm>

void ShatterEffect::Init()
{
    PostEffectBase::Init();

    shader_ = LoadPixelShader(
        (Application::PATH_SHADER + "Shatter.cso").c_str());
    constBuf_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * 3);

    const float sw = (float)Application::SCREEN_SIZE_X;
    const float sh = (float)Application::SCREEN_SIZE_Y;
    const float pw = sw / cols_;
    const float ph = sh / rows_;

    std::mt19937 rng(42);
    auto randf = [&](float lo, float hi) {
        return lo + (hi - lo) *
            (float)rng() / (float)(std::mt19937::max)();
        };

    for (int r = 0; r < rows_; r++) {
        for (int c = 0; c < cols_; c++) {
            Piece p;
            p.uvX = (c * pw) / sw;
            p.uvY = (r * ph) / sh;
            p.uvW = pw / sw;
            p.uvH = ph / sh;
            p.cx = c * pw + pw * 0.5f;
            p.cy = r * ph + ph * 0.5f;
            p.vx = 0.0f; p.vy = 0.0f;
            p.rot = 0.0f; p.rotV = 0.0f;
            p.alpha = 1.0f;
            p.delay = randf(0.0f, 0.5f);
            pieces_.push_back(p);

            p.shapeScale = 1.0f;
        }
    }

    // バッチ用バッファを最大サイズで確保
    int maxPieces = cols_ * rows_;
    batchVerts_.reserve(maxPieces * 4);
    batchIdx_.reserve(maxPieces * 6);
}

void ShatterEffect::Trigger(float clickX, float clickY)
{
    elapsedTime_ = 0.0f;
    isTriggered_ = true;
    isFinished_ = false;

    const float sw = (float)Application::SCREEN_SIZE_X;
    const float sh = (float)Application::SCREEN_SIZE_Y;
    const float pw = sw / cols_;
    const float ph = sh / rows_;

    const float maxDist = std::sqrt(sw * sw + sh * sh);

    std::mt19937 rng(std::random_device{}());
    auto randf = [&](float lo, float hi) {
        return lo + (hi - lo) *
            (float)rng() / (float)(std::mt19937::max)();
        };

    for (auto& p : pieces_) {
        float cx = p.uvX * sw + pw * 0.5f;
        float cy = p.uvY * sh + ph * 0.5f;
        float dx = cx - clickX;
        float dy = cy - clickY;
        float dist = std::sqrt(dx * dx + dy * dy);

        float nx, ny;
        if (dist < 1.0f) {
            float angle = randf(0.0f, 6.2832f);
            nx = std::cos(angle);
            ny = std::sin(angle);
        }
        else {
            nx = dx / dist;
            ny = dy / dist;
        }

        float distRatio = 1.0f - (std::min)(dist / (maxDist * 0.5f), 1.0f);
        p.shapeScale = 0.5f + distRatio * 3.0f;
        float speed = 150.0f + distRatio * 400.0f + randf(-30.0f, 30.0f);

        p.cx = cx;
        p.cy = cy;
        p.vx = nx * speed;
        p.vy = ny * speed - randf(20.0f, 80.0f);
        p.rot = 0.0f;
        p.rotV = randf(-2.0f, 2.0f) * distRatio;
        p.alpha = 1.0f;
        p.delay = (dist / maxDist) * 0.6f + randf(0.0f, 0.05f);

        float offset = 15.0f * p.shapeScale;

        // 左上
        p.cornerX[0] = -randf(0.0f, offset);
        p.cornerY[0] = -randf(0.0f, offset);

        // 右上
        p.cornerX[1] = randf(0.0f, offset);
        p.cornerY[1] = -randf(0.0f, offset);

        // 右下
        p.cornerX[2] = randf(0.0f, offset);
        p.cornerY[2] = randf(0.0f, offset);

        // 左下
        p.cornerX[3] = -randf(0.0f, offset);
        p.cornerY[3] = randf(0.0f, offset);
    }
}

void ShatterEffect::Update()
{
    InputManager& ins = InputManager::GetInstance();

    if (ins.IsTrgMouseRight()) {
        Vector2 mousePos = ins.GetMousePos();
        Trigger((int)mousePos.x, (int)mousePos.y);
    }

    const float dt = 1.0f / 60.0f;
    elapsedTime_ += dt;

    int alive = 0;
    for (auto& p : pieces_) {
        float pt = elapsedTime_ - p.delay;
        if (pt <= 0.0f) { alive++; continue; }

        p.vy += gravity_ * dt;
        p.cx += p.vx * dt;
        p.cy += p.vy * dt;
        p.rot += p.rotV * dt;
        p.alpha = (std::max)(0.0f, 1.0f - pt * 0.9f);

        if (p.alpha > 0.0f) alive++;
    }
    if (alive == 0) {
        isFinished_ = true;
        isTriggered_ = false;

    }
}

void ShatterEffect::Apply(int src, int dst)
{
    if (!isTriggered_) {
        SetDrawScreen(dst);
        DrawGraph(0, 0, src, false);
        return;
    }

    SetDrawScreen(dst);
    ClearDrawScreen();
    DrawGraph(0, 0, src, false);
    MV1SetUseOrigShader(true);
    SetUsePixelShader(shader_);
    SetUseTextureToShader(0, src);

    // 定数バッファは1回だけ更新・バインド
    SetConstantBuffer();
    UpdateShaderConstantBuffer(constBuf_);
    SetShaderConstantBuffer(constBuf_, DX_SHADERTYPE_PIXEL, CONSTANT_BUF_SLOT_BEGIN_PS);

    DrawPolygons();

    SetUseTextureToShader(0, -1);
    SetUsePixelShader(-1);
    MV1SetUseOrigShader(false);
}

void ShatterEffect::SetConstantBuffer()
{
    FLOAT4* cb = (FLOAT4*)GetBufferShaderConstantBuffer(constBuf_);

    // pieceUV.xyは頂点のsu/svから取得するため、ここはzw(サイズ)のみ意味を持つ
    cb[0] = { 0.0f, 0.0f, 1.0f / cols_, 1.0f / rows_ };
    cb[1] = { 0.0f, 0.0f, 0.0f, 1.0f };
    cb[2] = { (float)Application::SCREEN_SIZE_X,
              (float)Application::SCREEN_SIZE_Y, 0.0f, 0.0f };
}

void ShatterEffect::DrawPolygons()
{
    if (!isTriggered_) {
        DrawPolygonIndexed2DToShader(
            vertexs_, NUM_VERTEX, indexes_, NUM_POLYGON);
        return;
    }

    batchVerts_.clear();
    batchIdx_.clear();

    const float sw = (float)Application::SCREEN_SIZE_X;
    const float sh = (float)Application::SCREEN_SIZE_Y;
    const float pw = sw / cols_;
    const float ph = sh / rows_;

    WORD baseIdx = 0;

    for (auto& p : pieces_) {
        if (p.alpha <= 0.0f) continue;

        float hw = pw * 0.5f;
        float hh = ph * 0.5f;
        float cosR = std::cos(p.rot);
        float sinR = std::sin(p.rot);

        float lx[4] =
        {
            -hw + p.cornerX[0],
             hw + p.cornerX[1],
             hw + p.cornerX[2],
            -hw + p.cornerX[3]
        };

        float ly[4] =
        {
            -hh + p.cornerY[0],
            -hh + p.cornerY[1],
             hh + p.cornerY[2],
             hh + p.cornerY[3]
        };

        VERTEX2DSHADER v[4] = {};
        for (int i = 0; i < 4; i++) {
            v[i].rhw = 1.0f;
            v[i].dif = GetColorU8(255, 255, 255, (int)(p.alpha * 255));
            v[i].spc = GetColorU8(0, 0, 0, 0);
            v[i].su = p.uvX;  // ← 破片のUV原点Xを格納
            v[i].sv = p.uvY;  // ← 破片のUV原点Yを格納

            float rx = lx[i] * cosR - ly[i] * sinR + p.cx;
            float ry = lx[i] * sinR + ly[i] * cosR + p.cy;
            v[i].pos = VGet(rx, ry, 0.0f);
        }

        v[0].u = p.uvX;           v[0].v = p.uvY;
        v[1].u = p.uvX + p.uvW;   v[1].v = p.uvY;
        v[2].u = p.uvX + p.uvW;   v[2].v = p.uvY + p.uvH;
        v[3].u = p.uvX;           v[3].v = p.uvY + p.uvH;

        for (int i = 0; i < 4; i++) batchVerts_.push_back(v[i]);

        // インデックスはbaseIdxを基準にオフセット
        batchIdx_.push_back(baseIdx + 0);
        batchIdx_.push_back(baseIdx + 1);
        batchIdx_.push_back(baseIdx + 3);
        batchIdx_.push_back(baseIdx + 1);
        batchIdx_.push_back(baseIdx + 2);
        batchIdx_.push_back(baseIdx + 3);

        baseIdx += 4;
    }

    if (batchVerts_.empty()) return;

    int numPolygon = (int)(batchIdx_.size() / 3);
    DrawPolygonIndexed2DToShader(
        batchVerts_.data(), (int)batchVerts_.size(),
        batchIdx_.data(), numPolygon);
}