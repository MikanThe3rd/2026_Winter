#pragma once
#include "../../Common/Quaternion.h"
#include "../ActorBase.h"
#include "Stage.h"

class Ground : public ActorBase
{
public:

	// 重力の強さ
	static constexpr float DEFAULT_GRAVITY_POW = 10.0f;

	// 重力がかかる範囲
	static constexpr float DEFAULT_GRAVITY_RADIUS = 5000.0f;

	// ゲームオーバー範囲
	static constexpr float DEFAULT_DEAD_LENGTH = 1000.0f;

	// コンストラクタ
	Ground(const Stage::NAME& name, const Transform& transform);

	// デストラクタ
	~Ground(void);

	void Init(void) override;
	void Draw(void) override;

private:

	// ステージ名
	Stage::NAME name_;

	// 重力の強さ
	float gravityPow_;

	// 重力範囲
	float gravityRadius_;

	// ゲームーオーバー距離
	float deadLength_;

};

