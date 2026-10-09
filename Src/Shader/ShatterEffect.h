#pragma once
#include "PostEffectBase.h"
#include <vector>

class ShatterEffect : public PostEffectBase
{
public:
    void Init()   override;
    void Update() override;

    void Trigger(float clickX, float clickY);
    bool IsFinished() const { return isFinished_; }
    void SetGrid(int cols, int rows) { cols_ = cols; rows_ = rows; }

    void Apply(int src, int dst) override;

protected:
    void SetConstantBuffer() override;   // 破片1枚ぶんの定数バッファをセット
    void DrawPolygons()      override;   // 破片ループをここに書く

private:
    struct Piece {
        float uvX, uvY, uvW, uvH;
        float cx, cy;
        float vx, vy;
        float rot, rotV;
        float alpha;
        float delay;

        float cornerX[4];
        float cornerY[4];

        float shapeScale;
    };

    std::vector<Piece> pieces_;
    float elapsedTime_ = 0.0f;
    bool  isTriggered_ = false;
    bool  isFinished_ = false;
    int   cols_ = 8;
    int   rows_ = 6;
    float gravity_ = 700.0f;

    std::vector<VERTEX2DSHADER> batchVerts_;
    std::vector<WORD> batchIdx_;
};