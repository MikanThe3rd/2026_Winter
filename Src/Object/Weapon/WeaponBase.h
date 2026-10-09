#pragma once
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <unordered_set>
#include <DxLib.h>
#include "../Common/Transform.h"

class ResourceManager;
class Collider;
class Capsule;
class EnemyBase;

// 手のボーンからの持ち方
struct WeaponOffset
{
	VECTOR localPos;
	VECTOR rotEuler;
};

// 武器ごとのパラメーター
struct WeaponParam
{
	int damage = 1;                              // ダメージ
	float hitStart = 0.3f;                       // 判定開始(攻撃開始からの秒数)
	float hitEnd = 0.6f;                         // 判定終了
	VECTOR scale = { 1.0f, 1.0f, 1.0f };         // モデルの大きさ
	VECTOR capTop = { 0.0f, 80.0f, 0.0f };       // カプセル上端(ローカル)
	VECTOR capDown = { 0.0f, 30.0f, 0.0f };      // カプセル下端(ローカル)
	float capRadius = 20.0f;                     // カプセル半径
	WeaponOffset offset = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };  // 手からの持ち方
};

class WeaponBase
{
public:

	// コンストラクタ
	WeaponBase(void);

	// デストラクタ
	virtual ~WeaponBase(void);

	virtual void Init(void) = 0;
	virtual void Update(const Transform& playerTransform) = 0;
	virtual void Draw(void) = 0;

	// 更新関係
	virtual void UpdateTransform(const Transform& playerTransform) = 0;

	const Transform& GetTransform(void) const { return transform_; }

	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 攻撃状態を受け取り、判定の有効/無効を更新する
	void UpdateHit(bool isAttacking, float attackTime);

	// 今、攻撃判定が有効か
	bool IsHitActive(void) const { return hitActive_; }

	// 判定用カプセル
	const Capsule& GetCapsule(void) const { return *capsule_; }

	// 今の一振りで、すでに当てた敵か
	bool HasHit(const EnemyBase* enemy) const { return hitEnemies_.count(enemy) > 0; }

	// ヒットを記録して、効果を適用する(ダメージなど)
	void ApplyHit(EnemyBase& enemy);

protected:

	ResourceManager& resMng_;
	Transform transform_;

	// 武器ごとのパラメーター(派生クラスの Init で設定する)
	WeaponParam param_;

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;
	std::unique_ptr<Capsule> capsule_;

	// モデルとカプセルの共通初期化(派生クラスの Init から、param_ を設定してから呼ぶ)
	void InitCommon(int modelId);

	// 敵に当たったときの処理(既定はダメージのみ。武器ごとに上書き可)
	virtual void OnHit(EnemyBase& enemy);

private:

	bool hitActive_;

	// 今の一振りで当て済みの敵
	std::unordered_set<const EnemyBase*> hitEnemies_;
};
