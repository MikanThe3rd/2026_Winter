#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/EffectManager.h"
#include "../Object/Common/Capsule.h"
#include "../Object/Common/Collider.h"
#include "../Object/SkyDome.h"
#include "../Object/Stage.h"
#include "../Object/Player.h"
#include "../Object/Planet.h"
#include "../Object/Weapon/WeaponBase.h"
#include "../Object/Weapon/Sword.h"
#include "../Object/Enemy/EnemyBase.h"
#include "../Object/Enemy/EnemyManager.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	player_ = nullptr;
	weapon_ = nullptr;
	enemyManager_ = nullptr;
	skyDome_ = nullptr;
	stage_ = nullptr;
}

GameScene::~GameScene(void)
{
	effectManager_->Cleanup();
}

void GameScene::Init(void)
{

	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Init();

	// エネミー
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->SetTarget(&player_->GetTransform());
	enemyManager_->Init();

	// 武器
	weapon_ = std::make_unique<Sword>();
	weapon_->Init();

	// ステージ
	stage_ = std::make_unique<Stage>(*player_);
	stage_->Init();

	// 敵をステージに登録
	stage_->SetEnemyManager(enemyManager_.get());

	// ステージの初期設定
	stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(player_->GetTransform());
	skyDome_->Init();

	mainCamera.SetFollow(&player_->GetTransform());
	mainCamera.ChangeMode(Camera::MODE::FOLLOW);

	effectManager_ = std::make_unique<EffectManager>();
	effectManager_->Init();
}

void GameScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	skyDome_->Update();

	stage_->Update();

	player_->Update();

	enemyManager_->Update();

	// 武器の位置更新(アニメーション更新後の手のボーンから決まる)
	weapon_->Update(player_->GetTransform());
	weapon_->UpdateHit(player_->IsAttacking(), player_->GetAttackTime());

	// 当たり判定
	CollisionWeaponEnemy();

	effectManager_->Update();
}

void GameScene::Draw(void)
{
	// 背景
	skyDome_->Draw();
	stage_->Draw();

	player_->Draw();

	weapon_->Draw();

	enemyManager_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();

	effectManager_->Draw(mainScreen);
}

void GameScene::CollisionWeaponEnemy(void)
{
	// 判定が無効な間は何もしない
	if (!weapon_->IsHitActive())
	{
		return;
	}

	const Capsule& wc = weapon_->GetCapsule();

	for (const auto& e : enemyManager_->GetEnemies())
	{
		// 死亡済み、または今回の一振りで当て済みならスキップ
		if (!e->IsAlive() || weapon_->HasHit(e.get()))
		{
			continue;
		}

		const Capsule& ec = e->GetCapsule();
		int hit = HitCheck_Capsule_Capsule(
			wc.GetPosTop(), wc.GetPosDown(), wc.GetRadius(),
			ec.GetPosTop(), ec.GetPosDown(), ec.GetRadius());

		if (hit)
		{
			weapon_->ApplyHit(*e);
		}
	}
}
