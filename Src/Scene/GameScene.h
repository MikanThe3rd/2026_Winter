#pragma once
#include <memory>
#include <DxLib.h>
#include "SceneBase.h"

class Stage;
class SkyDome;
class Player;
class WeaponBase;
class EnemyManager;
class EffectManager;

class GameScene : public SceneBase
{

public:

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	// ステージ
	std::unique_ptr<Stage> stage_;

	// スカイドーム
	std::unique_ptr<SkyDome> skyDome_;

	// プレイヤー
	std::unique_ptr<Player> player_;

	// 武器
	std::unique_ptr<WeaponBase> weapon_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// エフェクトマネージャ
	std::unique_ptr<EffectManager> effectManager_;

	// 当たり判定(武器と敵)
	void CollisionWeaponEnemy(void);
};
