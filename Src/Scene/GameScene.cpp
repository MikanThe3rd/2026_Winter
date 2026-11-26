#include <vector>
#include <functional>
#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/EffectManager.h"
#include"../Manager/RunManager.h"
#include"../Scene/MenuScene.h"
#include "../Scene/RewardScene.h"    
#include "../Object/Common/Capsule.h"
#include "../Object/Common/Collider.h"
#include "../Object/SkyDome.h"
#include "../Object/Stage/Stage.h"
#include "../Object/Player.h"
#include "../Object/Weapon/WeaponBase.h"
#include "../Object/Weapon/WeaponManager.h"
#include "../Object/Enemy/EnemyBase.h"
#include "../Object/Enemy/EnemyManager.h"
#include "GameScene.h"

GameScene::GameScene(void)
{
	player_ = nullptr;
	weaponManager_ = nullptr;
	enemyManager_ = nullptr;
	skyDome_ = nullptr;
	stage_ = nullptr;
	shadowMapHandle_ = -1;
	specialTarget_ = nullptr;
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	// シャドウマップハンドルの作成
		shadowMapHandle_ = MakeShadowMap(4096, 4096);
	SetShadowMapLightDirection(shadowMapHandle_, { 0.3f, -0.7f, 0.8f });

	SetShadowMapDrawArea(shadowMapHandle_, VGet(-5000.0f, -70.0f, -6000.0f),
		VGet(6000.0f, 1500.0f, 8000.0f));

	RunManager& run = RunManager::GetInstance();

	player_ = std::make_unique<Player>();
	player_->Init();
	run.ApplyToPlayer(*player_);         

	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->SetTarget(&player_->GetTransform());
	if (run.IsRunning() && run.HasCurrentNode())
	{
		//  階層に応じた敵の強化
		enemyManager_->SetScale(run.GetEnemyHpRate(), run.GetEnemySpeedRate());
		enemyManager_->Init(run.GetCurrentNode().battleId);
	}
	else
	{
		enemyManager_->Init();   // GameScene 単体起動(デバッグ)時は従来どおり
	}

	// 武器
	weaponManager_ = std::make_unique<WeaponManager>();
	weaponManager_->Init();
	run.ApplyToWeapons(*weaponManager_);
	weaponManager_->SetDrawAll(true);

	// ステージ
	stage_ = std::make_unique<Stage>(*player_);
	stage_->Init();

	// 敵をステージに登録
	stage_->SetEnemyManager(enemyManager_.get());

	Stage::NAME stageName = Stage::NAME::CITY;
	if (run.IsRunning() && run.HasCurrentNode())
	{
		stageName = run.GetStageName();
	}
	stage_->ChangeStage(stageName);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(player_->GetTransform());
	skyDome_->Init();

	mainCamera.SetFollow(&player_->GetTransform());
	mainCamera.ChangeMode(Camera::MODE::FOLLOW);
}

void GameScene::Update(void)
{
	auto& ins = InputManager::GetInstance();

	skyDome_->Update();

	stage_->Update();

	int mouse = GetMouseInput();
	if (mouse & MOUSE_INPUT_LEFT ||
		ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER))
	{
		weaponManager_->Switch(WeaponManager::TYPE::SWORD);   // 左クリック: 剣
	}
	else if (mouse & MOUSE_INPUT_RIGHT ||
		ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::L_TRIGGER))
	{
		weaponManager_->Switch(WeaponManager::TYPE::CLAW);    // 右クリック: 爪
	}

	// 特殊攻撃の発生イベント
	SpecialAttackEvent();

	player_->Update();

	// 特殊攻撃の方向を更新(カメラ演出用)
	UpdateSpecialAttackDirection();

	enemyManager_->Update();

	// プレイヤーと敵、敵同士の重なりを解消する
	ResolveBodyOverlap();

	weaponManager_->Update(player_->GetTransform(),
		player_->IsAttacking(), player_->GetAttackTime(),
		player_->IsSpecialAttacking());

	// 当たり判定
	CollisionWeaponEnemy();

	CollisionEnemyWeaponPlayer();

	// ゲームクリア判定
	CheckGameClear();

	// ゲームオーバー判定
	CheckGameOver();


	// シーン遷移
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE) ||
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::MENU))
	{
		SceneManager::GetInstance().PushScene(new MenuScene());
	}
}

void GameScene::Draw(void)
{
	ShadowMap_DrawSetup(shadowMapHandle_);

	stage_->Draw();
	weaponManager_->Draw();
	enemyManager_->Draw();
	

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();

	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, shadowMapHandle_);

	// 背景
	skyDome_->Draw();

	stage_->Draw();

	player_->Draw();

	weaponManager_->Draw();

	enemyManager_->Draw();

	int mainScreen = SceneManager::GetInstance().GetMainScreen();


	player_->DrawHud();

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);
}

void GameScene::CollisionWeaponEnemy(void)
{

	WeaponBase& weapon = weaponManager_->GetActive();
	if (!weapon.IsHitActive()) return;

	if (player_->IsSpecialAttacking())
	{
		for (const auto& e : enemyManager_->GetEnemies())
		{
			if (e.get() == specialTarget_
				&& e->IsAlive()
				&& !weapon.HasHit(e.get()))
			{
				weapon.ApplyHit(*e);
			}
		}
		return;
	}

	// 通常攻撃: 武器カプセルと敵カプセルの接触で判定
	const Capsule& wc = weapon.GetCapsule();

	for (const auto& e : enemyManager_->GetEnemies())
	{
		// 死亡済み、または今回の一振りで当て済みならスキップ
		if (!e->IsAlive() || weapon.HasHit(e.get()))
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

void GameScene::CollisionEnemyWeaponPlayer(void)
{
	if (!player_->IsAlive() || player_->IsSpecialAttacking())
	{
		return;
	}

	const Capsule& pc = player_->GetCapsule();
	RunManager& run = RunManager::GetInstance();
	const int bonus = run.IsRunning() ? run.GetEnemyDamageBonus() : 0;

	for (const auto& e : enemyManager_->GetEnemies())
	{
		if (!e->IsAlive())
		{
			continue;
		}

		int dmg = e->CheckAttackHit(pc);
		if (dmg > 0)
		{
			player_->Damage(dmg + bonus);
		}
	}
}

void GameScene::CheckGameClear(void)
{
	for (const auto& e : enemyManager_->GetEnemies())
	{
		if (e->IsAlive())
		{
			return;
		}
	}

	RunManager& run = RunManager::GetInstance();
	if (!run.IsRunning())
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME_CLEAR);
		return;
	}

	run.StoreFromPlayer(*player_);

	// ボスは報酬なしでクリアへ
	if (run.IsBossNode())
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME_CLEAR);
		return;
	}

	// 通常 / 強敵は報酬画面を重ねる(GameScene は止まったまま背景に残る)
	const bool elite = (run.GetCurrentNode().type == RunManager::NODE_TYPE::ELITE);
	SceneManager::GetInstance().PushScene(new RewardScene(elite));
}

void GameScene::CheckGameOver(void)
{
	if (player_->IsAlive())
	{
		return;
	}
	SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME_OVER);
}

void GameScene::SpecialAttackEvent(void)
{
	// 特殊攻撃中は対象を固定する(途中で別の敵に変わらないように)
	if (player_->IsSpecialAttacking())
	{
		return;
	}
	specialTarget_ = nullptr;

	// プレイヤーに最も近いスタン中の敵を探す
	const VECTOR playerPos = player_->GetTransform().pos;
	float nearest = SPECIAL_ATTACK_RANGE;
	VECTOR targetPos = playerPos;

	for (const auto& e : enemyManager_->GetEnemies())
	{
		if (!e->IsAlive() || !e->IsStunned())
		{
			continue;
		}

		VECTOR pos = e->GetCapsule().GetPosDown();
		float dist = VSize(VSub(pos, playerPos));
		if (dist <= nearest)
		{
			nearest = dist;
			specialTarget_ = e.get();
			targetPos = pos;
		}
	}

	// 毎フレーム更新するので、スタンが解けたり離れたりすれば自動でfalseに戻る
	player_->SetSpecialAttack(specialTarget_ != nullptr, targetPos);
}

void GameScene::UpdateSpecialAttackDirection(void)
{
	// 特殊攻撃に入った瞬間だけ演出を開始する
	const bool isSpecial = player_->IsSpecialAttacking();
	if (isSpecial && !wasSpecialAttacking_)
	{
		mainCamera.StartEvent(Camera::EVENT_TYPE::BLOODSUCKING_CLAW,
			&player_->GetTransform(), SPECIAL_CAMERA_TIME);
	}
	wasSpecialAttacking_ = isSpecial;
}

void GameScene::ResolveBodyOverlap(void)
{
	// 押し戻し対象をまとめる(プレイヤー + 生存中の敵)
	struct Body
	{
		const Capsule* cap;
		float weight;                    // 押し戻される割合
		std::function<void(const VECTOR&)> move;
	};
	std::vector<Body> bodies;

	if (player_->IsAlive())
	{
		bodies.push_back({ &player_->GetCapsule(), PLAYER_PUSH_RATE,
			[this](const VECTOR& v) { player_->AddPos(v); } });
	}

	for (const auto& e : enemyManager_->GetEnemies())
	{
		if (!e->IsAlive()) continue;

		// 特殊攻撃の対象は演出中に動かさない
		const float w = (e.get() == specialTarget_ && player_->IsSpecialAttacking())
			? 0.0f : 1.0f;

		EnemyBase* ep = e.get();
		bodies.push_back({ &ep->GetCapsule(), w,
			[ep](const VECTOR& v) { ep->AddPos(v); } });
	}

	// 総当たりで判定
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		for (size_t j = i + 1; j < bodies.size(); ++j)
		{
			const Capsule& a = *bodies[i].cap;
			const Capsule& b = *bodies[j].cap;

			// カプセル(線分)同士の最短距離
			float dist = Segment_Segment_MinLength(
				a.GetPosTop(), a.GetPosDown(),
				b.GetPosTop(), b.GetPosDown());

			const float minDist = a.GetRadius() + b.GetRadius();
			if (dist >= minDist) continue;

			// 押し出す方向:水平(XZ)方向の中心間ベクトル
			VECTOR diff = VSub(b.GetPosDown(), a.GetPosDown());
			diff.y = 0.0f;
			float len = VSize(diff);
			if (len < 0.0001f)
			{
				diff = VGet(1.0f, 0.0f, 0.0f);   // 完全に重なった場合の逃げ道
				len = 1.0f;
			}
			const VECTOR dir = VScale(diff, 1.0f / len);

			// めり込んだ量
			const float depth = minDist - dist;

			// 重みで押し戻し量を配分
			const float wa = bodies[i].weight;
			const float wb = bodies[j].weight;
			const float sum = wa + wb;
			if (sum <= 0.0f) continue;

			bodies[i].move(VScale(dir, -depth * (wa / sum)));
			bodies[j].move(VScale(dir, depth * (wb / sum)));
		}
	}
}