#include <cmath>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../Common/Capsule.h"
#include "../Common/Collider.h"
#include "WeaponDatabase.h"
#include "Sword.h"

Sword::Sword(void)
{
}

Sword::~Sword(void)
{
}

void Sword::Init(void)
{
	// CSV からパラメーターを取得
	param_ = WeaponDatabase::GetInstance().Get("Sword");

	// モデルとカプセルの共通初期化
	InitCommon(resMng_.LoadModelDuplicate(ResourceManager::SRC::SWORD));
}

void Sword::Update(const Transform& playerTransform)
{
	UpdateTransform(playerTransform);
}

void Sword::Draw(void)
{
	// モデルの描画
	MV1DrawModel(transform_.modelId);

#ifdef _DEBUG
	// カプセルコライダの描画
	capsule_->Draw();

	// 攻撃判定が有効な間だけ表示(判定区間の調整用)
	if (IsHitActive())
	{
		DrawString(10, 10, "SWORD HIT ACTIVE", 0xff0000);
	}
#endif
}

static Quaternion MatrixToQuaternion(const MATRIX& m)
{
	double w, x, y, z;
	double trace = m.m[0][0] + m.m[1][1] + m.m[2][2];

	if (trace > 0.0)
	{
		double s = std::sqrt(trace + 1.0) * 2.0;
		w = 0.25 * s;
		x = (m.m[1][2] - m.m[2][1]) / s;
		y = (m.m[2][0] - m.m[0][2]) / s;
		z = (m.m[0][1] - m.m[1][0]) / s;
	}
	else if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2])
	{
		double s = std::sqrt(1.0 + m.m[0][0] - m.m[1][1] - m.m[2][2]) * 2.0;
		w = (m.m[1][2] - m.m[2][1]) / s;
		x = 0.25 * s;
		y = (m.m[0][1] + m.m[1][0]) / s;
		z = (m.m[2][0] + m.m[0][2]) / s;
	}
	else if (m.m[1][1] > m.m[2][2])
	{
		double s = std::sqrt(1.0 + m.m[1][1] - m.m[0][0] - m.m[2][2]) * 2.0;
		w = (m.m[2][0] - m.m[0][2]) / s;
		x = (m.m[0][1] + m.m[1][0]) / s;
		y = 0.25 * s;
		z = (m.m[1][2] + m.m[2][1]) / s;
	}
	else
	{
		double s = std::sqrt(1.0 + m.m[2][2] - m.m[0][0] - m.m[1][1]) * 2.0;
		w = (m.m[0][1] - m.m[1][0]) / s;
		x = (m.m[2][0] + m.m[0][2]) / s;
		y = (m.m[1][2] + m.m[2][1]) / s;
		z = 0.25 * s;
	}
	return Quaternion(w, x, y, z);
}

void Sword::UpdateTransform(const Transform& playerTransform)
{
	int playerModelId = playerTransform.modelId;
	int rightHandFrame = MV1SearchFrame(playerModelId, "mixamorig:RightHand");
	if (rightHandFrame == -1) return;

	MATRIX handMatrix = MV1GetFrameLocalWorldMatrix(playerModelId, rightHandFrame);

	VECTOR handPos = VGet(handMatrix.m[3][0], handMatrix.m[3][1], handMatrix.m[3][2]);

	// 手の回転のみ抽出(スケール除去)
	MATRIX handRotOnly = MGetIdent();
	for (int i = 0; i < 3; i++)
	{
		VECTOR row = VNorm(VGet(handMatrix.m[i][0], handMatrix.m[i][1], handMatrix.m[i][2]));
		handRotOnly.m[i][0] = row.x;
		handRotOnly.m[i][1] = row.y;
		handRotOnly.m[i][2] = row.z;
	}

	MATRIX offsetRot = Quaternion::Euler(param_.offset.rotEuler).ToMatrix();
	MATRIX rotMatrix = MMult(offsetRot, handRotOnly);

	VECTOR offset = VTransform(param_.offset.localPos, rotMatrix);
	VECTOR finalPos = VAdd(handPos, offset);

	// transform_ に書き戻す(コライダはこれを参照する)
	transform_.pos = finalPos;
	transform_.quaRot = MatrixToQuaternion(rotMatrix);
	transform_.Update();   // モデルの行列も transform_ から設定される
}
