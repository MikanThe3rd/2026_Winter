#include <cmath>
#include <string>
#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Capsule.h"
#include "Sister.h"

Sister::Sister(void)
{
}

Sister::~Sister(void)
{
}

void Sister::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::SISTER));
	transform_.scl = { 1.0f,1.0f,1.0f };
	transform_.pos = spawnData_.pos;
	transform_.quaRot = Quaternion::Euler(
		{ 0.0f, AsoUtility::Deg2RadF(spawnData_.rotY), 0.0f });
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	hpBarOffsetY_ = 200.0f;
	// アニメーションの設定
	InitAnimation();

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);
	// 初期状態
	ChangeState(STATE::SEARCH);
}

void Sister::Draw(void)
{
	EnemyBase::Draw();

	// 武器の描画
	if (weapon_)
	{
		weapon_->Draw();
	}
}

void Sister::InitAnimation(void)
{
	std::string path = Application::PATH_MODEL + "Sister/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Walk.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Attack.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::BACK, path + "Back.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::STUN, path + "Stun.mv1", 20.0f);

	animationController_->Play((int)ANIM_TYPE::IDLE);

}

void Sister::ChangeStateEngage(void)
{
	EnemyBase::ChangeStateEngage();

	// 入った直後は移動行動から始める(即攻撃を防ぎつつ、立ち止まらない)
	engageAction_ = (GetRand(1) == 0) ? ENGAGE_ACTION::BACK : ENGAGE_ACTION::APPROACH;
	engageTimer_ = ENGAGE_TIME_MIN;
}

void Sister::ChangeStateStun(void)
{
	EnemyBase::ChangeStateStun();

	stunTimer_ = STUN_TIME;

	animationController_->Play((int)ANIM_TYPE::STUN);
}

void Sister::ChangeStateAttack(void)
{
	EnemyBase::ChangeStateAttack();

	if (weapon_)
	{
		weapon_->StartAttack();
	}

	// ループ指定の引数は AnimationController::Play の仕様に合わせて調整
	animationController_->Play((int)ANIM_TYPE::ATTACK, false);
}

void Sister::UpdateSearch(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;

	if (target_ == nullptr)
	{
		return;
	}

	// プレイヤーとの水平距離
	VECTOR diff = VSub(target_->pos, transform_.pos);
	diff.y = 0.0f;

	// 範囲内に入ったら追従開始
	if (VSize(diff) < spawnData_.detectRange)
	{
		ChangeState(STATE::TRACKING);
		return;
	}

	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Sister::UpdateTracking(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;

	if (target_ == nullptr)
	{
		ChangeState(STATE::SEARCH);
		return;
	}

	// プレイヤーへの水平方向ベクトル(重力方向は無視)
	VECTOR diff = VSub(target_->pos, transform_.pos);
	diff.y = 0.0f;
	float dist = VSize(diff);

	// 離れすぎたら追従をやめる
	if (dist > spawnData_.loseRange)
	{
		ChangeState(STATE::SEARCH);
		return;
	}

	// 近すぎる場合は交戦状態へ(間合い調整・攻撃の抽選は ENGAGE 側で行う)
	if (dist <= spawnData_.stopRange)
	{
		ChangeState(STATE::ENGAGE);
		return;
	}

	VECTOR dir = VNorm(diff);

	// 移動量
	movePow_ = VScale(dir, spawnData_.speedMove);

	// プレイヤーの方向へ徐々に振り向く
	TurnToDir(dir);
	animationController_->Play((int)ANIM_TYPE::RUN);
}

void Sister::UpdateEngage(void)
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

void Sister::UpdateStun(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;
	if (--stunTimer_ <= 0)
	{
		ChangeState(STATE::TRACKING);
	}
}

void Sister::UpdateAttack(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;

	bool animEnd =
		animationController_->GetPlayType() == (int)ANIM_TYPE::ATTACK
		&& animationController_->IsEnd();

	if (weapon_ == nullptr || animEnd)
	{
		ChangeState(STATE::ENGAGE);
	}
}