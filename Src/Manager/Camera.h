#pragma once
#include <DxLib.h>
#include "../Common/Quaternion.h"
class Transform;

class Camera
{

public:

	// カメラスピード(度)
	static constexpr float SPEED = 1.0f;

	// カメラクリップ：NEAR
	static constexpr float CAMERA_NEAR = 10.0f;

	// カメラクリップ：NEAR
	static constexpr float CAMERA_FAR = 30000.0f;

	// カメラの初期座標
	static constexpr VECTOR DEFAULT_CAMERA_POS = { 0.0f, 100.0f, -500.0f };

	// 追従位置からカメラ位置までの相対座標
	static constexpr VECTOR LOCAL_F2C_POS = { 0.0f, 50.0f, -400.0f };

	// 追従位置から注視点までの相対座標
	static constexpr VECTOR LOCAL_F2T_POS = { 0.0f, 0.0f, 500.0f };

	// カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 0.0f * (DX_PI_F / 180.0f);

	// マウス感度
	static constexpr float MOUSE_SENSITIVITY = 0.003f;
	
	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FOLLOW,
		SELF_SHOT,
		EVENT
	};

	// イベントカメラの種類
	enum class EVENT_TYPE
	{
		NONE,
		BLOODSUCKING_CLAW,
	};

	Camera(void);
	~Camera(void);

	void Init(void);
	void Update(void);
	void SetBeforeDraw(void);
	void Draw(void);

	// カメラ位置
	VECTOR GetPos(void) const;
	// カメラの操作角度
	VECTOR GetAngles(void) const;
	// カメラの注視点
	VECTOR GetTargetPos(void) const;

	// カメラ角度
	Quaternion GetQuaRot(void) const;
	// X回転を抜いたカメラ角度
	Quaternion GetQuaRotOutX(void) const;


	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラ回転(ゲームパッド)
	void RotGamePad(bool isLimit);

	// カメラ回転(マウス)
	void RotMouse(bool isLimit);

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 追従対象の設定
	void SetFollow(const Transform* follow);

	// イベントカメラ開始/終了
	void StartEvent(EVENT_TYPE type, const Transform* target, float duration);
	void EndEvent(void);
	bool IsEventPlaying(void) const;

private:

	// カメラが追従対象とするTransform
	const Transform* followTransform_;

	// カメラモード
	MODE mode_;

	// カメラの位置
	VECTOR pos_;

	// カメラ角度(rad)
	VECTOR angles_;

	// X軸回転が無い角度
	Quaternion rotOutX_;

	// カメラ角度
	Quaternion rot_;

	// 注視点
	VECTOR targetPos_;

	// カメラの上方向
	VECTOR cameraUp_;

	// イベント用
	EVENT_TYPE eventType_;
	const Transform* eventTarget_;   
	float eventDuration_;            
	float eventTimer_;    
	int headFrameIdx_;

	// 復帰用
	MODE prevMode_;
	VECTOR prevAngles_;

	// 戻りブレンド用
	bool isBlendBack_;
	float blendTimer_;
	VECTOR blendStartPos_;
	VECTOR blendStartTarget_;
	static constexpr float BLEND_BACK_TIME = 0.4f;

	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(bool isLimit);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFollow(void);
	void SetBeforeDrawSelfShot(void);

	// イベントカメラの更新
	void SetBeforeDrawEvent(void);
	void UpdateSpecialMove(float t);

	// 顔の位置を取得する
	VECTOR GetFacePos(void) const;

	// 必殺技カメラの調整用定数
	static constexpr float SP_FACE_OFFSET_Y = 12.0f;  // 頭ボーンから顔の中心までの高さ(要調整)
	static constexpr float SP_FACE_HEIGHT = 160.0f; // 頭ボーンが取れない時の足元からの高さ
	static constexpr float SP_FRONT_ANGLE = 25.0f;  // 顔アップ時の角度(度)。0が真正面
	static constexpr float SP_BACK_ANGLE = 145.0f; // 突き出し時の角度(度)。180が真後ろ
	static constexpr float SP_DIST_START = 300.0f; // 開始距離
	static constexpr float SP_DIST_CLOSE = 110.0f; // 顔アップ距離
	static constexpr float SP_DIST_END = 320.0f; // 終了距離
	static constexpr float SP_HEIGHT_START = 60.0f;  // 寄り開始時の高さ(顔中心基準)
	static constexpr float SP_HEIGHT_CLOSE = 5.0f;  // 顔アップ時の高さ(大きいほど見下ろし)
	static constexpr float SP_HEIGHT_END = -90.0f; // 後ろ下の高さ
	static constexpr float SP_TARGET_UP_END = 40.0f;  // 終了時、注視点を上げる量

	// ---- タイミング(SpecialClaw.fbx: 74フレーム/30fps = 約2.47秒 を 0〜1 に正規化)----
	static constexpr float SP_CLOSE_END = 0.17f;    // ①の終了(手を引ききる f13)
	static constexpr float SP_ORBIT_START = 0.27f;    // ③の開始(突き出し開始 f20)
	static constexpr float SP_ORBIT_END = 0.41f;    // ③の終了(突き出し完了 f30)

	// ヘルパー
	static float SmoothStep(float t);
	VECTOR OrbitLocalPos(float angleDeg, float dist, float height) const;
};

