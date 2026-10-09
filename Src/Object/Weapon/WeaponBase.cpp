#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Capsule.h"
#include "../Common/Collider.h"
#include "../Enemy/EnemyBase.h"
#include "WeaponBase.h"

WeaponBase::WeaponBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	hitActive_(false)
{
}

WeaponBase::~WeaponBase(void)
{
}

void WeaponBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}

void WeaponBase::ClearCollider(void)
{
	colliders_.clear();
}

void WeaponBase::InitCommon(int modelId)
{
	// モデルの基本設定
	transform_.SetModel(modelId);
	transform_.scl = param_.scale;
	transform_.pos = { 0.0f, -30.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.Update();

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop(param_.capTop);
	capsule_->SetLocalPosDown(param_.capDown);
	capsule_->SetRadius(param_.capRadius);
}

void WeaponBase::UpdateHit(bool isAttacking, float attackTime)
{
	bool active = isAttacking
		&& attackTime >= param_.hitStart
		&& attackTime <= param_.hitEnd;

	// 無効から有効になった瞬間 = 新しい一振り。ヒット履歴をリセット
	if (active && !hitActive_)
	{
		hitEnemies_.clear();
	}
	hitActive_ = active;
}

void WeaponBase::ApplyHit(EnemyBase& enemy)
{
	hitEnemies_.insert(&enemy);
	OnHit(enemy);
}

void WeaponBase::OnHit(EnemyBase& enemy)
{
	enemy.Damage(param_.damage);
}
