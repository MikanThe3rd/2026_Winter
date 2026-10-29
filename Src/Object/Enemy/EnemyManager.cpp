#include <fstream>
#include <sstream>
#include <algorithm>
#include <DxLib.h>
#include "../Common/Collider.h"
#include "EnemyBase.h"
#include "Knight.h"
#include "EnemyManager.h"

namespace
{
	// 前後の空白を除去
	std::string Trim(const std::string& s)
	{
		const char* ws = " \t\r\n";
		size_t b = s.find_first_not_of(ws);
		if (b == std::string::npos)
		{
			return "";
		}
		size_t e = s.find_last_not_of(ws);
		return s.substr(b, e - b + 1);
	}
}

EnemyManager::EnemyManager(void)
	:
	target_(nullptr)
{
}

EnemyManager::~EnemyManager(void)
{
}

void EnemyManager::Init(const std::string& battleId)
{
	enemies_.clear();

	std::vector<EnemySpawnData> list;
	if (!LoadSpawnData(SPAWN_FILE_PATH, list))
	{
		OutputDebugStringA("[EnemyManager] スポーン設定ファイルを開けませんでした\n");
		return;
	}

	for (const auto& data : list)
	{
		// 戦闘IDが指定されていれば、その戦闘の行だけ生成する
		if (!battleId.empty() && data.battle != battleId)
		{
			continue;
		}
		Spawn(data);
	}

	if (enemies_.empty())
	{
		OutputDebugStringA(("[EnemyManager] 該当するエネミーがありません: battle=" + battleId + "\n").c_str());
	}
}

void EnemyManager::Update(void)
{
	for (auto& e : enemies_)
	{
		e->Update();
	}

	// 死亡した敵を取り除く
	enemies_.erase(
		std::remove_if(enemies_.begin(), enemies_.end(),
			[](const std::unique_ptr<EnemyBase>& e) { return !e->IsAlive(); }),
		enemies_.end());
}

void EnemyManager::Draw(void)
{
	for (auto& e : enemies_)
	{
		e->Draw();
	}
}

void EnemyManager::SetTarget(const Transform* target)
{
	target_ = target;
	for (auto& e : enemies_)
	{
		e->SetTarget(target_);
	}
}

void EnemyManager::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
	for (auto& e : enemies_)
	{
		e->AddCollider(collider);
	}
}

void EnemyManager::ClearCollider(void)
{
	colliders_.clear();
	for (auto& e : enemies_)
	{
		e->ClearCollider();
	}
}

bool EnemyManager::Spawn(const EnemySpawnData& data)
{
	auto enemy = Create(data.type);
	if (enemy == nullptr)
	{
		OutputDebugStringA(("[EnemyManager] 未対応のエネミー種別: " + data.type + "\n").c_str());
		return false;
	}

	// Init() の前にスポーン情報を渡す
	enemy->SetSpawnData(data);
	enemy->SetTarget(target_);
	for (const auto& c : colliders_)
	{
		enemy->AddCollider(c);
	}
	enemy->Init();

	enemy->EquipWeapon(data.weapon);

	enemies_.push_back(std::move(enemy));
	return true;
}

std::unique_ptr<EnemyBase> EnemyManager::Create(const std::string& type)
{
	// 新しいエネミーを作ったらここに追加する
	if (type == "Knight")
	{
		return std::make_unique<Knight>();
	}
	return nullptr;
}

bool EnemyManager::LoadSpawnData(const std::string& path, std::vector<EnemySpawnData>& out)
{
	std::ifstream ifs(path);
	if (!ifs)
	{
		return false;
	}

	std::string line;
	int lineNo = 0;
	while (std::getline(ifs, line))
	{
		lineNo++;

		// UTF-8 BOM を除去
		if (lineNo == 1 && line.size() >= 3
			&& static_cast<unsigned char>(line[0]) == 0xEF
			&& static_cast<unsigned char>(line[1]) == 0xBB
			&& static_cast<unsigned char>(line[2]) == 0xBF)
		{
			line.erase(0, 3);
		}

		line = Trim(line);

		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		std::vector<std::string> cols;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, ','))
		{
			cols.push_back(Trim(cell));
		}

		// ヘッダ行と、区切り用の空行(",,,,")は読み飛ばす
		if (cols.empty() || cols[0].empty() || cols[0] == "type")
		{
			continue;
		}

		if (cols.size() < 4)
		{
			OutputDebugStringA(("[EnemyManager] 列数が不足: " + std::to_string(lineNo) + " 行目\n").c_str());
			continue;
		}

		// 空欄ならデフォルトを維持して読む
		auto readOpt = [&](size_t idx, float& dst)
			{
				if (idx < cols.size() && !cols[idx].empty())
				{
					dst = std::stof(cols[idx]);
				}
			};

		try
		{
			EnemySpawnData d;
			d.type = cols[0];
			d.pos = VGet(std::stof(cols[1]), std::stof(cols[2]), std::stof(cols[3]));
			readOpt(4, d.rotY);
			readOpt(5, d.speedMove);
			readOpt(6, d.detectRange);
			readOpt(7, d.loseRange);
			readOpt(8, d.stopRange);

			// hp(整数)
			if (cols.size() > 9 && !cols[9].empty())
			{
				d.hp = std::stoi(cols[9]);
			}

			// 武器名
			if (cols.size() > 10)
			{
				d.weapon = cols[10];
			}

			// 所属する戦闘ID
			if (cols.size() > 11)
			{
				d.battle = cols[11];
			}

			out.push_back(d);
		}
		catch (const std::exception&)
		{
			OutputDebugStringA(("[EnemyManager] 数値の読み取りに失敗: " + std::to_string(lineNo) + " 行目\n").c_str());
		}
	}
	return true;
}