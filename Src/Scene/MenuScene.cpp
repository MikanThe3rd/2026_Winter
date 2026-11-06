#include"../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "MenuScene.h"

MenuScene::MenuScene(void)
	:
	selectNo_(0)
{
}

MenuScene::~MenuScene(void)
{
}

void MenuScene::Init(void)
{
}

void MenuScene::Update(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgDown(KEY_INPUT_W))
	{
		selectNo_--;

		if (selectNo_ < 0)
		{
			selectNo_ = 2;
		}
	}

	if (ins.IsTrgDown(KEY_INPUT_S))
	{
		selectNo_++;

		if (selectNo_ > 2)
		{
			selectNo_ = 0;
		}
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN))
	{
		switch (selectNo_)
		{
		case 0:
			// ゲームへ戻る
			SceneManager::GetInstance().PopScene();
			break;

		case 1:

			// タイトルへ戻る
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
			break;

		case 2:
			// ゲーム終了
			isEnd_ = true;
			break;
		}
	}
}

void MenuScene::Draw(void)
{
	// 背景を半透明で暗くする(文字より先に描く)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, 0x000000, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawFormatString(100, 100, 0xffffff, "ゲームに戻る");
	DrawFormatString(100, 120, 0xffffff, "タイトルへ戻る");
	DrawFormatString(100, 140, 0xffffff, "ゲーム終了");
	DrawFormatString(80, 100 + 20 * selectNo_, 0xffffff, "→");
}
