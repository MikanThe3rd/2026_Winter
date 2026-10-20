#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SHADER = "Data/Shader/";

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	// アプリケーションの初期設定
	SetWindowText("3DAction");

	// ウィンドウサイズ
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	InitEffekseer();

	// キー制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();
}

void Application::Run(void)
{
	auto& inputManager = InputManager::GetInstance();
	auto& sceneManager = SceneManager::GetInstance();

	// 計測の基準時刻を初期化
	frameStartTime_ = GetNowHiPerformanceCount();
	fpsCalcTime_ = frameStartTime_;

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		ScreenFlip();

		FpsControl();
	}
}

void Application::Destroy(void)
{
	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SceneManager::GetInstance().Destroy();

	// Effekseerを終了する。
	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	delete instance_;
}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
{
	isInitFail_ = false;
	isReleaseFail_ = false;

	frameStartTime_ = 0;
	fpsCount_ = 0;
	fpsCalcTime_ = 0;
	fps_ = 0.0f;
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}

void Application::FpsControl(void)
{
	// 目標時刻
	LONGLONG targetTime = frameStartTime_ + FRAME_TIME_US;

	while (true)
	{
		LONGLONG remain = targetTime - GetNowHiPerformanceCount();
		if (remain <= 2000)
		{
			break;
		}
		Sleep(1);
	}

	while (GetNowHiPerformanceCount() < targetTime)
	{
	}

	frameStartTime_ = targetTime;

	LONGLONG now = GetNowHiPerformanceCount();
	if (now - frameStartTime_ > FRAME_TIME_US * 2)
	{
		frameStartTime_ = now;
	}

	CalcFps();
}

void Application::CalcFps(void)
{
	fpsCount_++;

	// 1秒ごとにFPSを算出
	LONGLONG now = GetNowHiPerformanceCount();
	LONGLONG diff = now - fpsCalcTime_;
	if (diff >= 1000000)
	{
		fps_ = static_cast<float>(fpsCount_) * 1000000.0f / static_cast<float>(diff);
		fpsCount_ = 0;
		fpsCalcTime_ = now;
	}
}
