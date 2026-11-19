#pragma once
#include <map>
#include <DxLib.h>
#include "../Common/Transform.h"
#include "../../Manager/ResourceManager.h"

class Player;
class EnemyManager;

class Stage
{
public:

	// ステージの切り替え間隔
	static constexpr float TIME_STAGE_CHANGE = 1.0f;

	// ステージ名
	enum class NAME
	{
		FOREST,
		CITY,
		CHAPEL,
	};

	// コンストラクタ
	Stage(Player& player);

	// デストラクタ
	~Stage(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	// ステージ変更
	void ChangeStage(NAME type);

	// エネミーマネージャの登録(アクティブなステージのコライダが自動で設定される)
	void SetEnemyManager(EnemyManager* enemyManager);

private:

	// ステージ1つ分のデータ
	struct StageData
	{
		// 描画用モデル
		Transform drawTrans;

		// 当たり判定用モデル
		Transform colliderTrans;
	};

	// シングルトン参照
	ResourceManager& resMng_;

	Player& player_;

	// アクティブなステージ名
	NAME activeName_;

	// 全ステージ
	std::map<NAME, StageData> stages_;

	float step_;

	// 敵の管理(所有しない)
	EnemyManager* enemyManager_ = nullptr;

	// ステージ作成
	void MakeStage(NAME name,
		ResourceManager::SRC drawSrc,
		ResourceManager::SRC colliderSrc,
		const VECTOR& pos);

	// 敵にアクティブなステージのコライダを設定
	void SetColliderToEnemies(void);
};