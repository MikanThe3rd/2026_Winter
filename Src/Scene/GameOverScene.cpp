#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Object/SkyDome.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
{
}

GameOverScene::~GameOverScene(void)
{
}

void GameOverScene::Init(void)
{
	// îwåi
	spaceDomeTran_.pos = AsoUtility::VECTOR_ZERO;
	skyDome_ = std::make_unique<SkyDome>(spaceDomeTran_);
	skyDome_->Init();

	// íËì_ÉJÉÅÉâ
	mainCamera.ChangeMode(Camera::MODE::FIXED_POINT);
}

void GameOverScene::Update(void)
{
	// ÉVÅ[ÉìëJà⁄
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void GameOverScene::Draw(void)
{
	skyDome_->Draw();

	DrawFormatString(10, 10, GetColor(255, 255, 255), "GAME OVER!");
}
