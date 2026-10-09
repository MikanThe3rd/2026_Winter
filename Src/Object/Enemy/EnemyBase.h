#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <DxLib.h>
#include "../Common/Transform.h"
#include "EnemySpawnData.h"

class ResourceManager;
class Collider;
class Capsule;
class AnimationController;

class EnemyBase
{
public:

	enum class STATE
	{
		SEARCH,
		TRACKING,
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		WARP_PAUSE,
		FLY,
		FALLING,
		VICTORY
	};

	static constexpr int HP_BAR_WIDTH = 60;     // バーの幅
	static constexpr int HP_BAR_HEIGHT = 6;		// バーの高さ

	// コンストラクタ
	EnemyBase(void);

	// デストラクタ
	virtual ~EnemyBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void);
	virtual void Draw(void);

	virtual void InitAnimation(void) = 0;

	void Damage(int damage);

	bool IsAlive(void) const { return hp_ > 0; }
	int GetHp(void) const { return hp_; }
	int GetHpMax(void) const { return hpMax_; }

	const Transform& GetTransform(void) const { return transform_; }

	// 衝突用カプセルの取得
	const Capsule& GetCapsule(void) const { return *capsule_; }

	// 衝突判定に用いられるコライダ制御(Stage から呼ばれる)
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 追従対象の設定(所有しない)
	void SetTarget(const Transform* target) { target_ = target; }

	void SetSpawnData(const EnemySpawnData& data) {spawnData_ = data, hp_ = hpMax_ = data.hp;}

protected:

	EnemySpawnData spawnData_;


	// シングルトン参照
	ResourceManager& resMng_;

	Transform transform_;

	// 追従対象
	const Transform* target_;

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;
	std::unique_ptr<Capsule> capsule_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// 状態管理
	STATE state_;
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// 移動量
	VECTOR movePow_;

	// 重力・ジャンプによる移動量
	VECTOR jumpPow_;

	// 移動後の座標
	VECTOR movedPos_;

	// 衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// ヒットポイント
	int hp_;
	int hpMax_;

	// HPバーの表示位置オフセット
	float hpBarOffsetY_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateSearch(void);
	void ChangeStateTracking(void);

	virtual void UpdateSearch(void) = 0;
	virtual void UpdateTracking(void) = 0;

	// HPバーの描画
	virtual void DrawHpBar(void);

	// 重力量の計算
	void CalcGravityPow(void);

	// 衝突判定
	void Collision(void);
	void CollisionCapsule(void);
	void CollisionGravity(void);
};