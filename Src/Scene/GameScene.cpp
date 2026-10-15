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
#include "../Object/Weapon/WeaponManager.h"
#include "../Object/Enemy/EnemyBase.h"
#include "../Object/Weapon/EnemyWeapon.h"
#include "../Object/Enemy/EnemyManager.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	player_ = nullptr;
	weaponManager_ = nullptr;
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

	// 敵(CSV を読み込み、記載されたエネミーをスポーンさせる)
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->SetTarget(&player_->GetTransform());
	enemyManager_->Init();

	// 武器(武器を変えるのはここだけ)
	//weapon_ = std::make_unique<Sword>();
	weaponManager_ = std::make_unique<WeaponManager>();
	weaponManager_->Init();
	weaponManager_->SetDrawAll(true);

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

	int mouse = GetMouseInput();
	if (mouse & MOUSE_INPUT_LEFT)
	{
		weaponManager_->Switch(WeaponManager::TYPE::SWORD);   // 左クリック: 剣
	}
	else if (mouse & MOUSE_INPUT_RIGHT)
	{
		weaponManager_->Switch(WeaponManager::TYPE::CLAW);    // 右クリック: 爪
	}

	player_->Update();

	enemyManager_->Update();

	weaponManager_->Update(player_->GetTransform(),
		player_->IsAttacking(), player_->GetAttackTime());

	// 当たり判定
	CollisionWeaponEnemy();

	CollisionEnemyWeaponPlayer();

	effectManager_->Update();
}

void GameScene::Draw(void)
{
	// 背景
	skyDome_->Draw();

	stage_->Draw();

	player_->Draw();

	weaponManager_->Draw();

	enemyManager_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();

	effectManager_->Draw(mainScreen);

	player_->DrawHud();
}

void GameScene::CollisionWeaponEnemy(void)
{
	
	WeaponBase& weapon = weaponManager_->GetActive();
	if (!weapon.IsHitActive()) return;
	const Capsule& wc = weapon.GetCapsule();


	for (const auto& e : enemyManager_->GetEnemies())
	{
		// 死亡済み、または今回の一振りで当て済みならスキップ
		if (!e->IsAlive() ||weapon.HasHit(e.get()))
		{
			continue;
		}

		const Capsule& ec = e->GetCapsule();
		int hit = HitCheck_Capsule_Capsule(
			wc.GetPosTop(), wc.GetPosDown(), wc.GetRadius(),
			ec.GetPosTop(), ec.GetPosDown(), ec.GetRadius());

		if (hit)
		{
			weapon.ApplyHit(*e);
		}
	}
}

// 追加
void GameScene::CollisionEnemyWeaponPlayer(void)
{
	if (!player_->IsAlive())
	{
		return;
	}

	const Capsule& pc = player_->GetCapsule();

	for (const auto& e : enemyManager_->GetEnemies())
	{
		if (!e->IsAlive())
		{
			continue;
		}

		// 武器なしの敵、または判定が出ていない間はスキップ
		EnemyWeapon* w = e->GetWeapon();
		if (w == nullptr || !w->IsHitActive())
		{
			continue;
		}

		int hit = HitCheck_Capsule_Capsule(
			w->GetPosTop(), w->GetPosDown(), w->GetRadius(),
			pc.GetPosTop(), pc.GetPosDown(), pc.GetRadius());

		if (hit)
		{
			player_->Damage(w->GetData().damage);
			w->MarkHit();   // この一振りでは二度当たらない
		}
	}
}