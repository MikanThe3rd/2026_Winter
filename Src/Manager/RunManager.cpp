#include <algorithm>
#include <iterator>
#include <cmath>
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

	// 強化のリセット
	speedRate_ = 1.0f;
	std::fill(std::begin(weaponBonus_), std::end(weaponBonus_), 0);

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
	player.SetSpeedRate(speedRate_);
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

void RunManager::AddSpeedRate(float value)
{
	speedRate_ = (std::min)(SPEED_RATE_MAX, speedRate_ + value);
}

int RunManager::GetWeaponDamageBonus(WeaponManager::TYPE type) const
{
	return weaponBonus_[static_cast<int>(type)];
}

void RunManager::AddWeaponDamageBonus(WeaponManager::TYPE type, int value)
{
	weaponBonus_[static_cast<int>(type)] += value;
}

void RunManager::AddAllWeaponDamageBonus(int value)
{
	for (int& b : weaponBonus_)
	{
		b += value;
	}
}

void RunManager::ApplyToWeapons(WeaponManager& manager) const
{
	if (!isRunning_) return;

	for (int i = 0; i < static_cast<int>(WeaponManager::TYPE::MAX); ++i)
	{
		manager.SetDamageBonus(static_cast<WeaponManager::TYPE>(i), weaponBonus_[i]);
	}
}

// ---------------------------------------------------------------
// 戦闘報酬
// ---------------------------------------------------------------

std::vector<RunManager::Reward> RunManager::PickRewards(bool elite, int count)
{
	struct Def
	{
		REWARD_TYPE type;
		const char* name;
		int normal;     // 通常戦闘の値
		int strong;     // 強敵の値
	};

	// ★数値はここで調整(SPEED は % 、HEAL の 999 は全回復扱い)
	static const Def DEFS[] =
	{
		{ REWARD_TYPE::HP_MAX,       "最大HP強化",     2,  5   },
		{ REWARD_TYPE::HEAL,         "傷の手当て",     3,  999 },
		{ REWARD_TYPE::SPEED,        "移動速度強化",   10, 20  },
		{ REWARD_TYPE::SWORD_DAMAGE, "剣の強化",       1,  2   },
		{ REWARD_TYPE::CLAW_DAMAGE,  "爪の強化",       1,  2   },
		{ REWARD_TYPE::ALL_DAMAGE,   "全武器の強化",   0,  1   },   // 強敵のみ
	};

	// 候補(通常は value=0 の項目を除外)
	std::vector<int> pool;
	for (int i = 0; i < static_cast<int>(std::size(DEFS)); ++i)
	{
		if (!elite && DEFS[i].normal <= 0) continue;
		pool.push_back(i);
	}
	std::shuffle(pool.begin(), pool.end(), rng_);

	std::vector<Reward> ret;
	for (int i = 0; i < count && i < static_cast<int>(pool.size()); ++i)
	{
		const Def& d = DEFS[pool[i]];
		Reward r;
		r.type = d.type;
		r.elite = elite;
		r.value = elite ? d.strong : d.normal;
		r.name = d.name;

		switch (d.type)
		{
		case REWARD_TYPE::HP_MAX:
			r.desc = "最大HP +" + std::to_string(r.value); break;
		case REWARD_TYPE::HEAL:
			r.desc = (r.value >= 999) ? "HPを全回復" : "HPを " + std::to_string(r.value) + " 回復"; break;
		case REWARD_TYPE::SPEED:
			r.desc = "移動速度 +" + std::to_string(r.value) + "%"; break;
		case REWARD_TYPE::SWORD_DAMAGE:
			r.desc = "剣のダメージ +" + std::to_string(r.value); break;
		case REWARD_TYPE::CLAW_DAMAGE:
			r.desc = "爪のダメージ +" + std::to_string(r.value); break;
		case REWARD_TYPE::ALL_DAMAGE:
			r.desc = "全武器のダメージ +" + std::to_string(r.value); break;
		}
		ret.push_back(r);
	}
	return ret;
}

void RunManager::ApplyReward(const Reward& r)
{
	switch (r.type)
	{
	case REWARD_TYPE::HP_MAX:       AddHpMax(r.value); break;
	case REWARD_TYPE::HEAL:         Heal(r.value); break;
	case REWARD_TYPE::SPEED:        AddSpeedRate(r.value * 0.01f); break;
	case REWARD_TYPE::SWORD_DAMAGE: AddWeaponDamageBonus(WeaponManager::TYPE::SWORD, r.value); break;
	case REWARD_TYPE::CLAW_DAMAGE:  AddWeaponDamageBonus(WeaponManager::TYPE::CLAW, r.value); break;
	case REWARD_TYPE::ALL_DAMAGE:   AddAllWeaponDamageBonus(r.value); break;
	}
}

// ---------------------------------------------------------------
// 敵の強化(階層が進むほど強くなる)
// ---------------------------------------------------------------

float RunManager::GetEnemyHpRate(void) const
{
	// 1階層ごとに +10%
	return 1.0f + (std::max)(0, currentFloor_) * 0.10f;
}

float RunManager::GetEnemySpeedRate(void) const
{
	// 1階層ごとに +2%(上限 +30%)
	return (std::min)(1.3f, 1.0f + (std::max)(0, currentFloor_) * 0.02f);
}

int RunManager::GetEnemyDamageBonus(void) const
{
	// 4階層ごとに +1
	return (std::max)(0, currentFloor_) / 4;
}

Stage::NAME RunManager::GetStageName(void) const
{
	if (!HasCurrentNode())
	{
		return Stage::NAME::CITY;
	}

	// ボス
	if (IsBossNode())
	{
		return Stage::NAME::CHAPEL;
	}

	// 前半 / 後半(FLOOR_NUM の半分で分ける)
	if (currentFloor_ < FLOOR_NUM / 2)
	{
		return Stage::NAME::FOREST;
	}
	return Stage::NAME::CITY;
}
