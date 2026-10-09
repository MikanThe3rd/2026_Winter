#include <algorithm>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/AnimationController.h"
#include "../Common/Capsule.h"
#include "../Common/Collider.h"
#include "../Planet.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	state_(STATE::SEARCH),
	hp_(0),
	hpMax_(0),
	hpBarOffsetY_(0.0f)
{
	animationController_ = nullptr;
	capsule_ = nullptr;
	target_ = nullptr;

	movePow_ = AsoUtility::VECTOR_ZERO;
	jumpPow_ = AsoUtility::VECTOR_ZERO;
	movedPos_ = AsoUtility::VECTOR_ZERO;
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	// 状態管理
	stateChanges_.emplace(STATE::SEARCH, std::bind(&EnemyBase::ChangeStateSearch, this));
	stateChanges_.emplace(STATE::TRACKING, std::bind(&EnemyBase::ChangeStateTracking, this));
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Update(void)
{
	// 状態ごとの更新(movePow_ を決める)
	if (stateUpdate_)
	{
		stateUpdate_();
	}

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();

	// モデル制御更新
	transform_.Update();

	// アニメーション再生
	animationController_->Update();
}

void EnemyBase::Draw(void)
{
	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// HPバー
	DrawHpBar();
}

void EnemyBase::Damage(int damage)
{
	// 死亡済みなら何もしない
	if (!IsAlive())
	{
		return;
	}

	hp_ -= damage;

	if (hp_ < 0)
	{
		hp_ = 0;
	}
}

void EnemyBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}

void EnemyBase::ClearCollider(void)
{
	colliders_.clear();
}

void EnemyBase::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void EnemyBase::ChangeStateSearch(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateSearch, this);
}

void EnemyBase::ChangeStateTracking(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateTracking, this);
}

void EnemyBase::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		
		jumpPow_ = gravity;
	}
}

void EnemyBase::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	transform_.pos = movedPos_;
}

void EnemyBase::CollisionGravity(void)
{
	// ジャンプ量(重力)を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = Planet::DEFAULT_GRAVITY_POW;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));

	for (const auto& c : colliders_)
	{
		if (c.expired())
		{
			continue;
		}

		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// 重力量リセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
		}
	}
}

void EnemyBase::CollisionCapsule(void)
{
	// カプセルが未設定なら何もしない
	if (capsule_ == nullptr)
	{
		return;
	}

	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);

	// カプセルとの衝突判定
	for (const auto& c : colliders_)
	{
		if (c.expired())
		{
			continue;
		}

		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];

			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					// 法線方向に押し戻す
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
					// カプセルを移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;
				}
				break;
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void EnemyBase::DrawHpBar(void)
{
	if (!IsAlive() || hpMax_ <= 0)
	{
		return;
	}

	// 頭上のワールド座標 → 画面座標
	VECTOR worldPos = VAdd(transform_.pos,
		VScale(AsoUtility::DIR_U, hpBarOffsetY_));
	VECTOR sp = ConvWorldPosToScreenPos(worldPos);

	// カメラの後ろや描画範囲外なら描かない
	if (sp.z <= 0.0f || sp.z >= 1.0f)
	{
		return;
	}

	int w = HP_BAR_WIDTH;
	int h = HP_BAR_HEIGHT;
	int left = (int)sp.x - w / 2;
	int top = (int)sp.y;

	// 残量に応じて色を変える
	float rate = (float)hp_ / (float)hpMax_;
	int color = GetColor(60, 220, 60);
	if (rate <= 0.25f)
	{
		color = GetColor(230, 50, 50);
	}
	else if (rate <= 0.5f)
	{
		color = GetColor(240, 200, 40);
	}

	// 枠 → 背景 → 残量
	DrawBox(left - 1, top - 1, left + w + 1, top + h + 1, GetColor(0, 0, 0), TRUE);
	DrawBox(left, top, left + w, top + h, GetColor(70, 70, 70), TRUE);
	DrawBox(left, top, left + (int)(w * rate), top + h, color, TRUE);
}