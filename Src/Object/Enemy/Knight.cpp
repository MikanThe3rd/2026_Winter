#include <cmath>
#include <string>
#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Capsule.h"
#include "Knight.h"

Knight::Knight(void)
{
}

Knight::~Knight(void)
{
}

void Knight::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::KNIGHT));
	transform_.scl = { 0.12f,0.12f,0.12f };
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

void Knight::Draw(void)
{
	EnemyBase::Draw();

#ifdef _DEBUG
	// 検知範囲(球のワイヤーフレーム)を表示
	DrawSphere3D(transform_.pos, spawnData_.detectRange, 32, 0x00ff00, 0x00ff00, FALSE);
#endif
}

void Knight::InitAnimation(void)
{
	std::string path = Application::PATH_MODEL + "Knight/";
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Walk.mv1", 20.0f);

	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Knight::UpdateSearch(void)
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
	}

	animationController_->Play((int)ANIM_TYPE::IDLE);
}

void Knight::UpdateTracking(void)
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

	// 近すぎる場合は止まる
	if (dist <= spawnData_.stopRange)
	{
		return;
	}

	VECTOR dir = VNorm(diff);

	// 移動量
	movePow_ = VScale(dir, spawnData_.speedMove);

	// プレイヤーの方向へ徐々に振り向く
	float yaw = atan2f(dir.x, dir.z);
	Quaternion goal = Quaternion::AngleAxis((double)yaw, AsoUtility::AXIS_Y);
	transform_.quaRot = Quaternion::Slerp(transform_.quaRot, goal, ROT_RATE);
	animationController_->Play((int)ANIM_TYPE::RUN);
}