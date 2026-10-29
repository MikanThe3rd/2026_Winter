#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../Common/Transform.h"
#include "EnemySpawnData.h"

class EnemyBase;
class Collider;

class EnemyManager
{
public:

	// スポーン設定ファイル(全戦闘分を1ファイルにまとめる)
	static constexpr const char* SPAWN_FILE_PATH = "Data/CSV/EnemySpawn.csv";

	EnemyManager(void);
	~EnemyManager(void);

	// 設定ファイルを読み込み、battleId に一致する行のエネミーを生成する
	// battleId が空の場合は、全行を生成する(GameScene 単体起動のデバッグ用)
	void Init(const std::string& battleId = "");

	void Update(void);
	void Draw(void);

	// 追従対象の設定(全エネミーに反映。以降にスポーンする分にも適用)
	void SetTarget(const Transform* target);

	// 衝突判定用コライダ(全エネミーに反映。以降にスポーンする分にも適用)
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 1体スポーンさせる(ゲーム中の追加スポーンにも使用可)
	bool Spawn(const EnemySpawnData& data);

	// 全エネミー取得(プレイヤーとの当たり判定などに使用)
	const std::vector<std::unique_ptr<EnemyBase>>& GetEnemies(void) const { return enemies_; }

private:

	std::vector<std::unique_ptr<EnemyBase>> enemies_;

	// マネージャーが保持する共通設定
	const Transform* target_;
	std::vector<std::weak_ptr<Collider>> colliders_;

	// CSV 読み込み(全行を返す。絞り込みは Init で行う)
	bool LoadSpawnData(const std::string& path, std::vector<EnemySpawnData>& out);

	// 種別名からエネミーを生成
	std::unique_ptr<EnemyBase> Create(const std::string& type);
};