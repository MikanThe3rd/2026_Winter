#include <algorithm>
#include "../Object/Player.h"
#include "../Object/PlayerParam.h"
#include "RunManager.h"

namespace
{
	// 戦闘ID(Data/CSV/EnemySpawn.csv の battle 列と一致させる)
	const char* NORMAL_IDS[] = { "Normal01", "Normal02" };
	const char* ELITE_IDS[] = { "Elite01" };
	const char* BOSS_IDS[] = { "Boss01" };
}

RunManager& RunManager::GetInstance(void)
{
	static RunManager instance;
	return instance;
}

RunManager::RunManager(void)
	:
	isRunning_(false),
	currentFloor_(-1),
	currentIndex_(0),
	hp_(1),
	hpMax_(1),
	rng_(std::random_device{}())
{
}

void RunManager::StartNewRun(void)
{
	isRunning_ = true;
	currentFloor_ = -1;
	currentIndex_ = 0;
	path_.clear();

	const PlayerParam param = PlayerParam::Load();
	hpMax_ = param.hp;
	hp_ = param.hp;

	GenerateMap();
}

int RunManager::Random(int n)
{
	if (n <= 1) return 0;
	return std::uniform_int_distribution<int>(0, n - 1)(rng_);
}

RunManager::NODE_TYPE RunManager::DecideType(int floor)
{
	// 固定配置: 最初は戦闘、最後はボス、その手前は休憩(1つだけ)
	if (floor == 0) return NODE_TYPE::BATTLE;
	if (floor == FLOOR_NUM - 1) return NODE_TYPE::BOSS;
	if (floor == FLOOR_NUM - 2) return NODE_TYPE::REST;

	// それ以外の階層に休憩は出さない(戦闘・強敵・イベントのみ)
	const int r = Random(100);
	if (r < 60) return NODE_TYPE::BATTLE;
	if (r < 75) return floor >= 2 ? NODE_TYPE::ELITE : NODE_TYPE::BATTLE;
	return NODE_TYPE::EVENT;
}

std::string RunManager::PickBattleId(NODE_TYPE type)
{
	switch (type)
	{
	case NODE_TYPE::BATTLE:
		return NORMAL_IDS[Random(static_cast<int>(std::size(NORMAL_IDS)))];
	case NODE_TYPE::ELITE:
		return ELITE_IDS[Random(static_cast<int>(std::size(ELITE_IDS)))];
	case NODE_TYPE::BOSS:
		return BOSS_IDS[Random(static_cast<int>(std::size(BOSS_IDS)))];
	default:
		return "";
	}
}

void RunManager::GenerateMap(void)
{
	map_.assign(FLOOR_NUM, {});

	// ノード生成
	for (int f = 0; f < FLOOR_NUM; ++f)
	{
		// 休憩(ボス前)とボスは1ノードに固定。他の階層は FLOOR_WIDTH 個
		const int w = (f >= FLOOR_NUM - 2) ? 1 : FLOOR_WIDTH;
		for (int i = 0; i < w; ++i)
		{
			MapNode n;
			n.floor = f;
			n.index = i;
			n.type = DecideType(f);
			n.battleId = PickBattleId(n.type);
			map_[f].push_back(n);
		}
	}

	// 接続(各ノードは真上 + 50%で斜め上)
	for (int f = 0; f < FLOOR_NUM - 1; ++f)
	{
		const int nextW = GetFloorWidth(f + 1);
		std::vector<bool> hasIncoming(nextW, false);

		for (auto& n : map_[f])
		{
			if (nextW == 1)
			{
				n.next.push_back(0);
				hasIncoming[0] = true;
				continue;
			}

			n.next.push_back(n.index);
			hasIncoming[n.index] = true;

			if (Random(2) == 0)
			{
				const int k = n.index + (Random(2) == 0 ? -1 : 1);
				if (k >= 0 && k < nextW)
				{
					n.next.push_back(k);
					hasIncoming[k] = true;
				}
			}
			std::sort(n.next.begin(), n.next.end());
		}

		// どこからも辿れないノードを作らない
		for (int k = 0; k < nextW; ++k)
		{
			if (!hasIncoming[k])
			{
				auto& v = map_[f][(std::min)(k, GetFloorWidth(f) - 1)].next;
				v.push_back(k);
				std::sort(v.begin(), v.end());
			}
		}
	}
}

std::vector<int> RunManager::GetSelectableIndices(void) const
{
	std::vector<int> ret;
	if (!isRunning_) return ret;

	if (currentFloor_ < 0)
	{
		for (int i = 0; i < GetFloorWidth(0); ++i) ret.push_back(i);
		return ret;
	}
	if (currentFloor_ >= FLOOR_NUM - 1) return ret;
	return GetCurrentNode().next;
}

void RunManager::SelectNode(int index)
{
	currentFloor_++;
	currentIndex_ = index;
	path_.push_back(index);
}

bool RunManager::IsBossNode(void) const
{
	return HasCurrentNode() && GetCurrentNode().type == NODE_TYPE::BOSS;
}

void RunManager::ApplyToPlayer(Player& player) const
{
	if (!isRunning_) return;
	player.SetHpMax(hpMax_);
	player.SetHp(hp_);
}

void RunManager::StoreFromPlayer(const Player& player)
{
	hpMax_ = player.GetHpMax();
	hp_ = (std::max)(1, player.GetHp());
}

void RunManager::Heal(int value)
{
	hp_ = (std::min)(hpMax_, hp_ + value);
}

void RunManager::Damage(int value)
{
	hp_ = (std::max)(1, hp_ - value);
}

void RunManager::AddHpMax(int value)
{
	hpMax_ += value;
	hp_ += value;
}