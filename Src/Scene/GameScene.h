#pragma once
#include <memory>
#include <DxLib.h>
#include "SceneBase.h"

class Stage;
class SkyDome;
class Player;
class WeaponManager;
class EnemyManager;
class EffectManager;
class EnemyBase;

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
	std::unique_ptr<WeaponManager> weaponManager_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// エフェクトマネージャ
	std::unique_ptr<EffectManager> effectManager_;

	// 当たり判定(武器と敵)
	void CollisionWeaponEnemy(void);

	// 追加: 当たり判定(敵の武器とプレイヤー)
	void CollisionEnemyWeaponPlayer(void);

	// ゲームクリア判定
	void CheckGameClear(void);

	// ゲームオーバー判定
	void CheckGameOver(void);

	//特殊攻撃の発生イベント
	void SpecialAttackEvent(void);

	// 特殊攻撃の対象(スタン中の敵)。比較にのみ使い、参照はしない
	const EnemyBase* specialTarget_ = nullptr;

	// 特殊攻撃が発生する距離
	static constexpr float SPECIAL_ATTACK_RANGE = 400.0f;

	// 特殊攻撃の開始検出用
	bool wasSpecialAttacking_ = false;

	// 特殊攻撃のイベントカメラ時間(秒)。アニメの長さに合わせる
	static constexpr float SPECIAL_CAMERA_TIME = 2.47f;

	// 特殊攻撃が始まった瞬間の演出(カメラなど)
	void UpdateSpecialAttackDirection(void);
};