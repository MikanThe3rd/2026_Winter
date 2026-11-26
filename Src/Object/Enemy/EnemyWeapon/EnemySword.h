#pragma once
#include <string>
#include <DxLib.h>
#include "EnemyAttackBase.h"

// 武器1種類分の性能・見た目・判定の設定
struct WeaponData
{
	std::string modelPath;      // 武器モデル(.mv1)
	std::string handFrameName;  // 装着先のフレーム名(モデルによって異なる)
	VECTOR localPos;            // 手からのオフセット
	VECTOR localRot;            // 手からの回転(度)
	float  scale;               // 武器モデルの拡大率
	int    damage;              // プレイヤーに与えるダメージ
	float  range;               // 攻撃を開始できる距離
	int    cooldownFrames;      // 攻撃開始から次の攻撃までの待ち時間
	int    attackFrames;        // 攻撃モーション全体の長さ
	int    hitStartFrame;       // 判定が出始めるフレーム
	int    hitEndFrame;         // 判定が終わるフレーム
	float  hitLength;           // 武器ローカルのY方向に伸ばす判定の長さ
	float  hitRadius;           // 判定の半径(ワールド単位)
};

class EnemySword : public EnemyAttackBase
{
public:

	EnemySword(const WeaponData& data, int ownerModelId);
	~EnemySword(void) override;

	void Update(const Context& ctx) override;
	void Draw(void) override;

	bool CanAttack(float dist) const override;
	void StartAttack(void) override;
	void EndAttack(void) override { attackTimer_ = 0; }
	bool IsAttacking(void) const override { return attackTimer_ > 0; }

	// 当たっていれば damage を返し、この一振りでは二度当たらないようにする
	int CheckHitPlayer(const Capsule& player) override;

	const WeaponData& GetData(void) const { return data_; }

private:

	// 判定が有効か(攻撃中・判定期間内・今回の一振りで未ヒット)
	bool IsHitActive(void) const;

	WeaponData data_;
	int ownerModelId_;
	int modelId_;
	int frameIdx_;
	int cooldownTimer_;
	int attackTimer_;

	bool   hasHit_;
	VECTOR posTop_;
	VECTOR posDown_;
};