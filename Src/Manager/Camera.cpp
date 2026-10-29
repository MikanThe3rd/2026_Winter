#include <math.h>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include <memory>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include"../Manager/ResourceManager.h"
#include "../Object/Common/Transform.h"
#include "Camera.h"

Camera::Camera(void)
	:
	resMng_(ResourceManager::GetInstance())
{
	angles_ = VECTOR();
	cameraUp_ = VECTOR();
	mode_ = MODE::NONE;
	pos_ = AsoUtility::VECTOR_ZERO;
	targetPos_ = AsoUtility::VECTOR_ZERO;
	followTransform_ = nullptr;
	prevMode_ = MODE::FOLLOW;
	prevAngles_ = VECTOR();
	eventTarget_ = nullptr;
	eventType_ = EVENT_TYPE::NONE;
	eventDuration_ = 0.0f;
	eventTimer_ = 0.0f;
	headFrameIdx_ = -1;
	isBlendBack_ = false;
	blendTimer_ = 0.0f;
	blendStartPos_ = AsoUtility::VECTOR_ZERO;
	blendStartTarget_ = AsoUtility::VECTOR_ZERO;
	cursorImg_ = -1;
	cursorPosX_ = 0;
	cursorPosY_ = 0;
}

Camera::~Camera(void)
{
	if (cursorImg_ != -1)
	{
		DeleteGraph(cursorImg_);
	}
}

void Camera::Init(void)
{
	ChangeMode(MODE::FIXED_POINT);

	cursorImg_ = LoadGraph("Data/Image/Cursor.png");
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	case Camera::MODE::EVENT:
		SetBeforeDrawEvent();
		break;
	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		pos_,
		targetPos_,
		cameraUp_
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::Draw(void)
{
	DrawCursor();
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

VECTOR Camera::GetPos(void) const
{
	return pos_;
}

VECTOR Camera::GetAngles(void) const
{
	return angles_;
}

VECTOR Camera::GetTargetPos(void) const
{
	return targetPos_;
}

Quaternion Camera::GetQuaRot(void) const
{
	return rot_;
}

Quaternion Camera::GetQuaRotOutX(void) const
{
	return rotOutX_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, pos_));
}

void Camera::RotGamePad(bool isLimit)
{

}

void Camera::RotMouse(bool isLimit)
{
	const int CENTER_X = Application::SCREEN_SIZE_X / 2;
	const int CENTER_Y = Application::SCREEN_SIZE_Y / 2;

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// 画面中央からの移動量を直接計算
	float deltaX = static_cast<float>(mouseX - CENTER_X);
	float deltaY = static_cast<float>(mouseY - CENTER_Y);

	// 移動量がほぼ 0 でなければカメラの角度を更新
	if (deltaX != 0.0f || deltaY != 0.0f)
	{
		// X軸
		angles_.x += deltaY * MOUSE_SENSITIVITY;

		// Y軸
		angles_.y += deltaX * MOUSE_SENSITIVITY;
	}
	// 角度制限（上下回転）
	if (isLimit)
	{
		if (angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
		if (angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}
	SetMousePoint(CENTER_X, CENTER_Y);
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetMouseDispFlag(FALSE);
		break;
	case Camera::MODE::FOLLOW:
		break;
	case Camera::MODE::EVENT:
		break;
	}

}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	pos_ = DEFAULT_CAMERA_POS;

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

	angles_.x = AsoUtility::Deg2RadF(30.0f);
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	rot_ = Quaternion();

}

void Camera::SyncFollow(void)
{

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// 重力の方向制御に従う
	// 正面から設定されたY軸分、回転させる
	rotOutX_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// 正面から設定されたX軸分、回転させる
	rot_ = rotOutX_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点(通常重力でいうところのY値を追従対象と同じにする)
	localPos = rotOutX_.PosAxis(LOCAL_F2T_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = rot_.PosAxis(LOCAL_F2C_POS);
	pos_ = VAdd(pos, localPos);

	// カメラの上方向
	cameraUp_ = AsoUtility::DIR_U;

}

void Camera::ProcessRot(bool isLimit)
{

	auto& ins = InputManager::GetInstance();

	// 左Altを押していない間はマウスが中央に固定
	if (!ins.IsNew(KEY_INPUT_LALT))
	{
		SetMouseDispFlag(FALSE);
		RotMouse(isLimit);
	}
	else
	{
		SetMouseDispFlag(FALSE);
		// 方向回転によるXYZの移動(キーボード)

	}
}

void Camera::SetBeforeDrawFixedPoint(void)
{

}

void Camera::SetBeforeDrawFollow(void)
{
	// カメラ操作
	RotGamePad(true);
	ProcessRot(true);

	// 追従対象との相対位置を同期
	SyncFollow();

	// イベントからの戻りブレンド
	if (isBlendBack_)
	{
		blendTimer_ += 1.0f / 60.0f;
		float r = blendTimer_ / BLEND_BACK_TIME;
		if (r >= 1.0f)
		{
			isBlendBack_ = false;
		}
		else
		{
			// イージング(ease-out)
			r = 1.0f - (1.0f - r) * (1.0f - r);
			pos_ = VAdd(VScale(blendStartPos_, 1.0f - r), VScale(pos_, r));
			targetPos_ = VAdd(VScale(blendStartTarget_, 1.0f - r), VScale(targetPos_, r));
		}
	}
}

void Camera::SetBeforeDrawSelfShot(void)
{
}

void Camera::StartEvent(EVENT_TYPE type, const Transform* target, float duration)
{
	// すでにイベント中なら無視(prevMode_の上書き防止)
	if (mode_ == MODE::EVENT || target == nullptr || duration <= 0.0f)
	{
		return;
	}

	// 元の状態を退避(ChangeModeのSetDefaultで消えるため)
	prevMode_ = mode_;
	prevAngles_ = angles_;

	ChangeMode(MODE::EVENT);

	eventType_ = type;
	eventTarget_ = target;
	eventDuration_ = duration;
	eventTimer_ = 0.0f;
	isBlendBack_ = false;

	// 頭ボーンを探す(Mixamoのボーン名。見つからなければ-1で固定高さにフォールバック)
	headFrameIdx_ = MV1SearchFrame(target->modelId, "mixamorig:Head");
}

void Camera::EndEvent(void)
{
	blendStartPos_ = pos_;
	blendStartTarget_ = targetPos_;
	blendTimer_ = 0.0f;
	isBlendBack_ = true;

	// 戻り先が不正ならFOLLOWにする
	if (prevMode_ == MODE::EVENT || prevMode_ == MODE::NONE)
	{
		prevMode_ = MODE::FOLLOW;
	}
	mode_ = prevMode_;
	angles_ = prevAngles_;
}

bool Camera::IsEventPlaying(void) const
{
	return mode_ == MODE::EVENT;
}

void Camera::SetBeforeDrawEvent(void)
{
	// 固定フレームなら1/60、可能ならデルタタイムを使う
	const float dt = 1.0f / 60.0f;
	eventTimer_ += dt;

	float t = eventTimer_ / eventDuration_;
	if (t >= 1.0f)
	{
		EndEvent();
		return;
	}

	switch (eventType_)
	{
	case EVENT_TYPE::BLOODSUCKING_CLAW:
		UpdateSpecialMove(t);
		break;
	}

	cameraUp_ = AsoUtility::DIR_U;
}

void Camera::UpdateSpecialMove(float t)
{
	Quaternion q = eventTarget_->quaRot;

	// 顔の位置(ワールド)
	VECTOR face = GetFacePos();

	VECTOR localPos;     // 顔中心基準のカメラ相対位置
	float targetUp = 0.0f;

	if (t < SP_CLOSE_END)
	{
		// ① 手を引いている間に、斜め前から顔へ寄る
		float r = SmoothStep(t / SP_CLOSE_END);
		float dist = SP_DIST_START + (SP_DIST_CLOSE - SP_DIST_START) * r;
		float height = SP_HEIGHT_START + (SP_HEIGHT_CLOSE - SP_HEIGHT_START) * r;
		localPos = OrbitLocalPos(SP_FRONT_ANGLE, dist, height);
	}
	else if (t < SP_ORBIT_START)
	{
		// ② 引ききって溜めている間、顔アップ(少しだけ寄り続ける)
		float r = (t - SP_CLOSE_END) / (SP_ORBIT_START - SP_CLOSE_END);
		float dist = SP_DIST_CLOSE - 15.0f * r;
		localPos = OrbitLocalPos(SP_FRONT_ANGLE, dist, SP_HEIGHT_CLOSE);
	}
	else
	{
		// ③ 突き出しに合わせて斜め後ろ下へ(ORBIT_END以降は終了位置で止まる)
		float r = SmoothStep((t - SP_ORBIT_START) / (SP_ORBIT_END - SP_ORBIT_START));
		float startDist = SP_DIST_CLOSE - 15.0f;

		// ④ 突き出し維持中は少しだけ動かし続ける
		float drift = (t > SP_ORBIT_END) ? (t - SP_ORBIT_END) / (1.0f - SP_ORBIT_END) : 0.0f;

		float angle = SP_FRONT_ANGLE + (SP_BACK_ANGLE - SP_FRONT_ANGLE) * r + 10.0f * drift;
		float dist = startDist + (SP_DIST_END - startDist) * r;
		float height = SP_HEIGHT_CLOSE + (SP_HEIGHT_END - SP_HEIGHT_CLOSE) * r;
		localPos = OrbitLocalPos(angle, dist, height);

		// 注視点を少し上げて「見上げる」構図にする
		targetUp = SP_TARGET_UP_END * r;
	}

	// キャラの向きを反映してワールド座標へ
	pos_ = VAdd(face, q.PosAxis(localPos));
	targetPos_ = VAdd(face, VGet(0.0f, targetUp, 0.0f));
}

VECTOR Camera::GetFacePos(void) const
{
	const Transform* tr = eventTarget_;

	// 頭ボーンが取れていればアニメに追従する
	if (headFrameIdx_ >= 0)
	{
		VECTOR head = MV1GetFramePosition(tr->modelId, headFrameIdx_);
		return VAdd(head, VGet(0.0f, SP_FACE_OFFSET_Y, 0.0f));
	}

	// フォールバック:キャラ位置+固定の高さ
	return VAdd(tr->pos, VGet(0.0f, SP_FACE_HEIGHT, 0.0f));
}

float Camera::SmoothStep(float t)
{
	t = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
	return t * t * (3.0f - 2.0f * t);
}

// キャラ中心の円柱座標 → ローカル座標(キャラの正面が+Z)
VECTOR Camera::OrbitLocalPos(float angleDeg, float dist, float height) const
{
	float rad = AsoUtility::Deg2RadF(angleDeg);
	return VGet(sinf(rad) * dist, height, cosf(rad) * dist);
}

void Camera::DrawCursor(void)
{
	// カーソルを出すのは「固定カメラ中」か「FOLLOW中にAltでカーソルを解放している間」
	bool isShow = false;
	if (mode_ == MODE::FIXED_POINT)
	{
		isShow = true;
	}
	else if (mode_ == MODE::FOLLOW &&
		InputManager::GetInstance().IsNew(KEY_INPUT_LALT))
	{
		isShow = true;
	}

	if (!isShow || cursorImg_ == -1)
	{
		return;
	}

	GetMousePoint(&cursorPosX_, &cursorPosY_);

	// 画像の先端(CURSOR_TIP)がマウス座標に来るように描画
	DrawRotaGraph2(
		cursorPosX_, cursorPosY_,
		static_cast<int>(CURSOR_TIP_X), static_cast<int>(CURSOR_TIP_Y),
		CURSOR_SCALE, 0.0,
		cursorImg_, true);
}