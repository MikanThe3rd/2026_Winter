#pragma once
#include <vector>
#include <string>
#include <random>

class Player;

// 1回の冒険(ラン)の状態をシーンをまたいで保持する
// マップ構造・現在地・プレイヤーのHPなど
class RunManager
{
public:

	// ノード種別
	enum class NODE_TYPE
	{
		BATTLE,   // 通常戦闘
		ELITE,    // 強敵
		EVENT,    // イベント
		REST,     // 休憩
		BOSS,     // ボス
	};

	struct MapNode
	{
		NODE_TYPE type = NODE_TYPE::BATTLE;
		int floor = 0;
		int index = 0;
		std::vector<int> next;        // 次の階層で進めるノードの index
		std::string battleId;         // 戦闘ノードの戦闘ID(EnemySpawn.csv の battle 列)
	};

	// 階層数と1階層あたりの最大ノード数(最終階はボス1つ)
	static constexpr int FLOOR_NUM = 10;
	static constexpr int FLOOR_WIDTH = 3;

	static RunManager& GetInstance(void);

	// 新しい冒険を始める(マップ生成・HP初期化)
	void StartNewRun(void);

	// 冒険中か(GameScene 単体起動のデバッグ対策)
	bool IsRunning(void) const { return isRunning_; }

	// ---- マップ ----
	const std::vector<std::vector<MapNode>>& GetMap(void) const { return map_; }
	int GetFloorWidth(int floor) const { return static_cast<int>(map_[floor].size()); }

	// 現在地(まだ何も選んでいなければ floor = -1)
	int GetCurrentFloor(void) const { return currentFloor_; }
	int GetCurrentIndex(void) const { return currentIndex_; }
	const MapNode& GetCurrentNode(void) const { return map_[currentFloor_][currentIndex_]; }
	bool HasCurrentNode(void) const { return currentFloor_ >= 0; }

	// いま選べるノードの index 一覧
	std::vector<int> GetSelectableIndices(void) const;

	// 次の階層のノードを選んで進む
	void SelectNode(int index);

	// 辿ってきた経路(その階層で選んだノードの index。未到達なら -1)
	int GetPathIndex(int floor) const
	{
		return (floor >= 0 && floor < static_cast<int>(path_.size())) ? path_[floor] : -1;
	}

	// 現在ノードがボスか
	bool IsBossNode(void) const;

	// ---- プレイヤー状態の引き継ぎ ----
	void ApplyToPlayer(Player& player) const;        // 戦闘開始時
	void StoreFromPlayer(const Player& player);      // 戦闘終了時

	int GetHp(void) const { return hp_; }
	int GetHpMax(void) const { return hpMax_; }
	void Heal(int value);
	void Damage(int value);       // 死なない(最低HP1)
	void AddHpMax(int value);

	// 乱数 [0, n)
	int Random(int n);

private:

	RunManager(void);
	~RunManager(void) = default;
	RunManager(const RunManager&) = delete;
	RunManager& operator=(const RunManager&) = delete;

	void GenerateMap(void);
	NODE_TYPE DecideType(int floor);
	std::string PickBattleId(NODE_TYPE type);

	bool isRunning_;
	std::vector<std::vector<MapNode>> map_;
	int currentFloor_;
	int currentIndex_;
	std::vector<int> path_;

	int hp_;
	int hpMax_;

	std::mt19937 rng_;
};