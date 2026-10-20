#pragma once
#include <string>
#include <Windows.h>

class Application
{

public:

	// スクリーンサイズ
	static constexpr int SCREEN_SIZE_X = 1024;
	static constexpr int SCREEN_SIZE_Y = 640;

	// FPS関連
	static constexpr int FPS = 60;
	static constexpr LONGLONG FRAME_TIME_US = 1000000 / FPS;

	// データパス関連
	//-------------------------------------------
	static const std::string PATH_IMAGE;
	static const std::string PATH_MODEL;
	static const std::string PATH_EFFECT;
	static const std::string PATH_SHADER;
	//-------------------------------------------

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static Application& GetInstance(void);

	// 初期化
	void Init(void);

	// ゲームループの開始
	void Run(void);

	// リソースの破棄
	void Destroy(void);

	// 初期化成功／失敗の判定
	bool IsInitFail(void) const;

	// 解放成功／失敗の判定
	bool IsReleaseFail(void) const;

private:

	// 静的インスタンス
	static Application* instance_;

	// 初期化失敗
	bool isInitFail_;

	// 解放失敗
	bool isReleaseFail_;

	// FPS制御用
	LONGLONG frameStartTime_;   // フレーム開始時刻(マイクロ秒)
	int fpsCount_;              // 1秒間のフレーム数カウント
	LONGLONG fpsCalcTime_;      // FPS計測の基準時刻
	float fps_;                 // 計測したFPS

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	Application(void);
	Application(const Application& manager) = default;
	~Application(void) = default;

	// Effekseerの初期化
	void InitEffekseer(void);

	// FPS制御(目標時間まで待機)
	void FpsControl(void);

	// FPS計測
	void CalcFps(void);

};
