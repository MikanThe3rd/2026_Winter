#include <map>
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/ResourceManager.h"
#include "../Player.h"
#include "../Enemy/EnemyManager.h"
#include "../Common/Collider.h"
#include "../Common/Transform.h"
#include "Stage.h"

Stage::Stage(Player& player)
	: resMng_(ResourceManager::GetInstance()), player_(player)
{
	activeName_ = NAME::CITY;
	step_ = 0.0f;
}

Stage::~Stage(void)
{
	stages_.clear();
}

void Stage::Init(void)
{
	// 街のステージ
	MakeStage(NAME::CITY,
		ResourceManager::SRC::CITY,
		ResourceManager::SRC::CITY_COLLISION,
		{ 0.0f, -100.0f, 0.0f });


	MakeStage(NAME::FOREST,
		ResourceManager::SRC::FOREST,
		ResourceManager::SRC::FOREST,
		{ 0.0f, -100.0f, 0.0f });

	step_ = -1.0f;
}

void Stage::Update(void)
{
	// ギミックなし
}

void Stage::Draw(void)
{
	// アクティブなステージの描画用モデルだけ描く(当たり判定用は描かない)
	auto it = stages_.find(activeName_);
	if (it == stages_.end())
	{
		return;
	}
	MV1DrawModel(it->second.drawTrans.modelId);
}

void Stage::ChangeStage(NAME type)
{
	auto it = stages_.find(type);
	if (it == stages_.end())
	{
		return;
	}

	activeName_ = type;

	// ステージの当たり判定をプレイヤーに設定
	player_.ClearCollider();
	player_.AddCollider(it->second.colliderTrans.collider);

	// 敵にも設定
	SetColliderToEnemies();

	step_ = TIME_STAGE_CHANGE;
}

void Stage::MakeStage(NAME name,
	ResourceManager::SRC drawSrc,
	ResourceManager::SRC colliderSrc,
	const VECTOR& pos)
{
	StageData data;

	// 描画用
	data.drawTrans.SetModel(resMng_.LoadModelDuplicate(drawSrc));
	data.drawTrans.scl = { 1.0f, 1.0f, 1.0f };
	data.drawTrans.quaRot = Quaternion();
	data.drawTrans.pos = pos;
	data.drawTrans.Update();

	// 当たり判定用
	data.colliderTrans.SetModel(resMng_.LoadModelDuplicate(colliderSrc));
	data.colliderTrans.scl = { 1.0f, 1.0f, 1.0f };
	data.colliderTrans.quaRot = Quaternion();
	data.colliderTrans.pos = pos;
	data.colliderTrans.MakeCollider(Collider::TYPE::STAGE);
	data.colliderTrans.Update();

	stages_.emplace(name, std::move(data));
}

void Stage::SetEnemyManager(EnemyManager* enemyManager)
{
	enemyManager_ = enemyManager;

	// すでにステージが有効なら、その場でコライダを設定
	SetColliderToEnemies();
}

void Stage::SetColliderToEnemies(void)
{
	auto it = stages_.find(activeName_);
	if (it == stages_.end() || enemyManager_ == nullptr)
	{
		return;
	}

	// 全エネミー(以降のスポーン分も含む)に反映される
	enemyManager_->ClearCollider();
	enemyManager_->AddCollider(it->second.colliderTrans.collider);
}