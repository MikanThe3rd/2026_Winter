#include<string>
#include "../../Utility/AsoUtility.h"
#include "EnemyWeapon.h"

EnemyWeapon::EnemyWeapon(const WeaponData& data, int ownerModelId)
	:
	data_(data),
	ownerModelId_(ownerModelId),
	modelId_(-1),
	frameIdx_(-1),
	cooldownTimer_(0),
	attackTimer_(0),
	hasHit_(false),
	posTop_(VGet(0.0f, 0.0f, 0.0f)),
	posDown_(VGet(0.0f, 0.0f, 0.0f))
{
	modelId_ = MV1LoadModel(data_.modelPath.c_str());
	if (modelId_ == -1)
	{
		OutputDebugStringA(("[EnemyWeapon] モデル読み込み失敗: " + data_.modelPath + "\n").c_str());
	}

	frameIdx_ = MV1SearchFrame(ownerModelId_, data_.handFrameName.c_str());
	if (frameIdx_ < 0)
	{
		OutputDebugStringA(("[EnemyWeapon] フレームが見つかりません: " + data_.handFrameName + "\n").c_str());

		// 敵モデルが持つフレーム名を全部出す(手のフレーム名を探すため)
		int num = MV1GetFrameNum(ownerModelId_);
		for (int i = 0; i < num; i++)
		{
			OutputDebugStringA(("  frame[" + std::to_string(i) + "] " + MV1GetFrameName(ownerModelId_, i) + "\n").c_str());
		}
	}
}

EnemyWeapon::~EnemyWeapon(void)
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
	}
}

void EnemyWeapon::Update(void)
{
	if (cooldownTimer_ > 0) { cooldownTimer_--; }
	if (attackTimer_ > 0) { attackTimer_--; }

	if (frameIdx_ < 0 || modelId_ == -1)
	{
		return;
	}

	// 手のフレームのワールド行列
	MATRIX hand = MV1GetFrameLocalWorldMatrix(ownerModelId_, frameIdx_);

	// 位置だけ取り出す
	VECTOR handPos = VGet(hand.m[3][0], hand.m[3][1], hand.m[3][2]);

	// 回転のみ抽出(スケール除去)
	MATRIX handRot = MGetIdent();
	for (int i = 0; i < 3; i++)
	{
		VECTOR row = VNorm(VGet(hand.m[i][0], hand.m[i][1], hand.m[i][2]));
		handRot.m[i][0] = row.x;
		handRot.m[i][1] = row.y;
		handRot.m[i][2] = row.z;
	}

	// オフセット回転(度 → ラジアン。順序は従来どおり X→Y→Z)
	MATRIX offsetRot = MGetRotX(AsoUtility::Deg2RadF(data_.localRot.x));
	offsetRot = MMult(offsetRot, MGetRotY(AsoUtility::Deg2RadF(data_.localRot.y)));
	offsetRot = MMult(offsetRot, MGetRotZ(AsoUtility::Deg2RadF(data_.localRot.z)));
	MATRIX rotMatrix = MMult(offsetRot, handRot);

	// オフセット位置(手の向き基準)
	VECTOR finalPos = VAdd(handPos, VTransform(data_.localPos, rotMatrix));

	// 拡大 → 回転 → 平行移動
	MATRIX world = MMult(MGetScale(VGet(data_.scale, data_.scale, data_.scale)), rotMatrix);
	world.m[3][0] = finalPos.x;
	world.m[3][1] = finalPos.y;
	world.m[3][2] = finalPos.z;
	MV1SetMatrix(modelId_, world);

	// 判定カプセル(以下は従来どおり)
	posDown_ = VTransform(VGet(0.0f, 0.0f, 0.0f), world);
	posTop_ = VTransform(VGet(0.0f, data_.hitLength, 0.0f), world);
}

void EnemyWeapon::Draw(void)
{
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}

#ifdef _DEBUG
	int color = IsHitActive() ? 0xff0000 : 0x808080;
	DrawCapsule3D(posTop_, posDown_, data_.hitRadius, 8, color, color, FALSE);

	// 攻撃開始からの経過フレームを表示(振りに合わせて hitStart/hitEnd を決める用)
	if (attackTimer_ > 0)
	{
		DrawFormatString(10, 10, 0xffffff, "attack elapsed: %d / %d",
			data_.attackFrames - attackTimer_, data_.attackFrames);
	}
#endif
}

bool EnemyWeapon::CanAttack(float dist) const
{
	return cooldownTimer_ <= 0 && attackTimer_ <= 0 && dist <= data_.range;
}

void EnemyWeapon::StartAttack(void)
{
	attackTimer_ = data_.attackFrames;
	cooldownTimer_ = data_.cooldownFrames;
	hasHit_ = false;    // 新しい一振りなので当たり済みをリセット
}

bool EnemyWeapon::IsHitActive(void) const
{
	if (attackTimer_ <= 0 || hasHit_)
	{
		return false;
	}
	int elapsed = data_.attackFrames - attackTimer_;
	return elapsed >= data_.hitStartFrame && elapsed < data_.hitEndFrame;
}