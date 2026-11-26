#pragma once
#include <DxLib.h>

class Capsule;

// 敵の攻撃の共通インターフェース
class EnemyAttackBase
{
public:

	// 毎フレーム、敵から渡される情報
	struct Context
	{
		VECTOR ownerPos;    // 攻撃する敵の位置
		VECTOR targetPos;   // プレイヤーの位置
	};

	virtual ~EnemyAttackBase(void) = default;

	// アニメーション更新後に呼ぶ
	virtual void Update(const Context& ctx) = 0;
	virtual void Draw(void) = 0;

	// 攻撃を開始できるか(dist は敵とプレイヤーの水平距離)
	virtual bool CanAttack(float dist) const = 0;
	virtual void StartAttack(void) = 0;

	// 攻撃モーションの中断(弾など、発射済みのものは消さない)
	virtual void EndAttack(void) = 0;
	virtual bool IsAttacking(void) const = 0;

	// プレイヤーに当たったか判定し、与えるダメージを返す(当たらなければ 0)
	// 多段ヒット防止は各攻撃クラスの内部で管理する
	virtual int CheckHitPlayer(const Capsule& player) = 0;
};