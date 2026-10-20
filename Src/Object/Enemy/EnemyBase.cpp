#include <algorithm>
#include <cmath>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/AnimationController.h"
#include "../Common/Capsule.h"
#include "../Common/Collider.h"
#include "../Planet.h"
#include "EnemyWeapon/EnemySword.h"           
#include "EnemyWeapon/EnemyWeaponDatabase.h"    
#include "EnemyBase.h"

EnemyBase::EnemyBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	state_(STATE::SEARCH),
	hp_(0),
	hpMax_(0),
	hpBarOffsetY_(0.0f),
	engageAction_(ENGAGE_ACTION::BACK),     // 追加
	engageTimer_(0),                        // 追加
	stunTimer_(0),                          // 追加
	stunTriggered_(false)                   // 追加
{
	animationController_ = nullptr;
	capsule_ = nullptr;
	target_ = nullptr;
	weapon_ = nullptr;      // 追加

	movePow_ = AsoUtility::VECTOR_ZERO;
	jumpPow_ = AsoUtility::VECTOR_ZERO;
	movedPos_ = AsoUtility::VECTOR_ZERO;
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	// 状態管理
	stateChanges_.emplace(STATE::SEARCH, std::bind(&EnemyBase::ChangeStateSearch, this));
	stateChanges_.emplace(STATE::TRACKING, std::bind(&EnemyBase::ChangeStateTracking, this));
	stateChanges_.emplace(STATE::ENGAGE, std::bind(&EnemyBase::ChangeStateEngage, this));   // 追加
	stateChanges_.emplace(STATE::STUN, std::bind(&EnemyBase::ChangeStateStun, this));       // 追加
	stateChanges_.emplace(STATE::ATTACK, std::bind(&EnemyBase::ChangeStateAttack, this));   // 追加
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

	// 追加: 武器の更新(アニメーション反映後の手の位置を使うため、必ずこの位置)
	if (weapon_)
	{
		weapon_->Update();
	}
}

void EnemyBase::Draw(void)
{
	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// 追加: 武器の描画
	if (weapon_)
	{
		weapon_->Draw();
	}

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

	// 残りHPがしきい値以下になったら、1回だけスタンする(倒れた場合は対象外)
	if (!stunTriggered_ && hp_ > 0 && hp_ <= STUN_HP_THRESHOLD)
	{
		stunTriggered_ = true;
		ChangeState(STATE::STUN);
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

void EnemyBase::EquipWeapon(const std::string& weaponName)
{
	// 空なら武器なし
	if (weaponName.empty())
	{
		return;
	}

	const WeaponData* data = FindWeaponData(weaponName);
	if (data == nullptr)
	{
		OutputDebugStringA(("[EnemyBase] 未登録の武器名: " + weaponName + "\n").c_str());
		return;
	}

	weapon_ = std::make_unique<EnemySword>(*data, transform_.modelId);
}

bool EnemyBase::TryStartAttack(float dist)
{
	if (weapon_ == nullptr || !weapon_->CanAttack(dist))
	{
		return false;
	}

	ChangeState(STATE::ATTACK);
	return true;
}

void EnemyBase::ChangeState(STATE state)
{
	// 攻撃状態を抜けるときは武器の攻撃も止める
	if (state_ == STATE::ATTACK && state != STATE::ATTACK && weapon_)
	{
		weapon_->EndAttack();
	}
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

void EnemyBase::ChangeStateEngage(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateEngage, this);

	// 入った直後は移動行動から始める(即攻撃を防ぎつつ、立ち止まらない)
	engageAction_ = (GetRand(1) == 0) ? ENGAGE_ACTION::BACK : ENGAGE_ACTION::APPROACH;
	engageTimer_ = ENGAGE_TIME_MIN;
}

void EnemyBase::ChangeStateStun(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateStun, this);

	stunTimer_ = STUN_TIME;

	animationController_->Play((int)ANIM_TYPE::STUN);
}

void EnemyBase::UpdateStun(void)
{
	// 行動不能
	movePow_ = AsoUtility::VECTOR_ZERO;

	if (--stunTimer_ <= 0)
	{
		// 復帰後は追従へ。近ければ Knight 側で ENGAGE に遷移する
		ChangeState(STATE::TRACKING);
	}
}

void EnemyBase::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateAttack, this);

	if (weapon_)
	{
		weapon_->StartAttack();
	}

	// ループ指定の引数は AnimationController::Play の仕様に合わせて調整
	animationController_->Play((int)ANIM_TYPE::ATTACK, false);
}

void EnemyBase::UpdateEngage(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;

	if (target_ == nullptr)
	{
		ChangeState(STATE::SEARCH);
		return;
	}

	VECTOR diff = VSub(target_->pos, transform_.pos);
	diff.y = 0.0f;
	float dist = VSize(diff);

	// 離れたら追従/探索に戻る
	if (dist > spawnData_.loseRange)
	{
		ChangeState(STATE::SEARCH);
		return;
	}
	if (dist > spawnData_.stopRange + ENGAGE_EXIT_MARGIN)
	{
		ChangeState(STATE::TRACKING);
		return;
	}

	// 常にプレイヤーの方を向く
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	if (dist > 0.001f)
	{
		dir = VScale(diff, 1.0f / dist);
		TurnToDir(dir);
	}

	// 行動の抽選
	if (--engageTimer_ <= 0)
	{
		PickEngageAction(dist);
		if (state_ != STATE::ENGAGE)
		{
			return;     // 攻撃に遷移した
		}
	}

	// 行動の実行
	switch (engageAction_)
	{
	case ENGAGE_ACTION::BACK:
		// 下がりすぎない(離脱ラインの手前で止める)
		if (dist < spawnData_.stopRange + ENGAGE_EXIT_MARGIN * 0.5f)
		{
			// 向きは上で常にプレイヤーへ向けているので、逆方向へ動くだけで後退になる
			movePow_ = VScale(dir, -spawnData_.speedMove);
			animationController_->Play((int)ANIM_TYPE::BACK);
		}
		else
		{
			engageTimer_ = 0;   // 次フレームで再抽選
		}
		break;

	case ENGAGE_ACTION::APPROACH:
		// 寄りすぎない
		if (dist > spawnData_.stopRange * 0.5f)
		{
			movePow_ = VScale(dir, spawnData_.speedMove);
			animationController_->Play((int)ANIM_TYPE::RUN);
		}
		else
		{
			engageTimer_ = 0;
		}
		break;

	default:
		break;
	}
}

void EnemyBase::PickEngageAction(float dist)
{
	int r = GetRand(99);    // DxLib: 0〜99

	if (r < RATE_ATTACK)
	{
		// 攻撃できなければ(武器なし/クールダウン中)下がるか寄るかを選ぶ
		if (TryStartAttack(dist))
		{
			return;
		}
		engageAction_ = (GetRand(1) == 0) ? ENGAGE_ACTION::BACK : ENGAGE_ACTION::APPROACH;
	}
	else if (r < RATE_BACK)
	{
		engageAction_ = ENGAGE_ACTION::BACK;
	}
	else
	{
		engageAction_ = ENGAGE_ACTION::APPROACH;
	}

	engageTimer_ = ENGAGE_TIME_MIN + GetRand(ENGAGE_TIME_RAND);
}

void EnemyBase::TurnToDir(const VECTOR& dir)
{
	float yaw = atan2f(dir.x, dir.z);
	Quaternion goal = Quaternion::AngleAxis((double)yaw, AsoUtility::AXIS_Y);
	transform_.quaRot = Quaternion::Slerp(transform_.quaRot, goal, TURN_RATE);
}

void EnemyBase::UpdateAttack(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;

	// 攻撃アニメーションが最後まで再生されたら交戦に戻る
	bool animEnd =
		animationController_->GetPlayType() == (int)ANIM_TYPE::ATTACK
		&& animationController_->IsEnd();

	if (weapon_ == nullptr || animEnd)
	{
		ChangeState(STATE::ENGAGE);   // 変更: TRACKING → ENGAGE
	}
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