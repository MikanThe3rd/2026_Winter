#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <DxLib.h>
#include "ActorBase.h"
#include "PlayerParam.h"

class AnimationController;
class Collider;
class Capsule;

class Player : public ActorBase
{

public:

	// 移動速度・回転時間・ジャンプ力・HPは CSV(PlayerParam)から設定される

	// ジャンプ受付時間
	static constexpr float TIME_JUMP_IN = 0.5f;

	// HPバー(プレイヤーの頭上に追従)
	static constexpr int HP_BAR_WIDTH = 80;
	static constexpr int HP_BAR_HEIGHT = 8;
	static constexpr float HP_BAR_OFFSET_Y = 190.0f;   // 足元から頭上までの高さ

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		ATTACK,
		BLOODSUCKING,
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		ATTACK,
		CLAW,
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 衝突用カプセルの取得
	const Capsule& GetCapsule(void) const;

	// 攻撃中か
	bool IsAttacking(void) const { return state_ == STATE::ATTACK || state_ == STATE::BLOODSUCKING; }

	// 攻撃開始からの経過時間(秒)
	float GetAttackTime(void) const { return stepAttack_; }

	// HP
	int GetHp(void) const { return hp_; }
	int GetHpMax(void) const { return hpMax_; }
	bool IsAlive(void) const { return hp_ > 0; }

	// ダメージを受ける
	void Damage(int damage);

	// HPバーの描画(頭上に追従)。3D描画の後に呼ぶ
	void DrawHud(void);

private:

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// 状態管理
	STATE state_;
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// 移動スピード
	float speed_;

	// 移動方向
	VECTOR moveDir_;

	// 移動量
	VECTOR movePow_;

	// 移動後の座標
	VECTOR movedPos_;

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ量
	VECTOR jumpPow_;

	// ジャンプ判定
	bool isJump_;

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;
	std::unique_ptr<Capsule> capsule_;

	// 衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// 丸影
	int imgShadow_;

	// 攻撃開始からの経過時間
	float stepAttack_;

	// ステータス(CSV から読み込み)
	PlayerParam playerParam_;

	// HP
	int hp_;
	int hpMax_;

	void InitAnimation(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateAttack(void);
	void ChangeStateBloodsucking(void);

	// 更新ステップ
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateAttack(void);
	void UpdateBloodsucking(void);
	
	// 描画系
	void DrawShadow(void);

	// 操作
	void ProcessMove(void);
	void ProcessJump(void);
	void ProcessAttack(void);
	void ProcessAttackBloodsucking(void);

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);
	void CollisionCapsule(void);

	// 移動量の計算
	void CalcGravityPow(void);

	// 着地モーション終了
	bool IsEndLanding(void);

};
