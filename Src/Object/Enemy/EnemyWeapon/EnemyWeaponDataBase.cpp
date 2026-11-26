#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <DxLib.h>
#include "EnemyWeaponDatabase.h"

namespace
{
	const char* const WEAPON_CSV_PATH = "Data/CSV/EnemyWeapon.csv";
	const char* const MAGIC_CSV_PATH = "Data/CSV/EnemyMagic.csv";

	// 列数(name を含む)
	constexpr size_t WEAPON_COLS = 18;
	constexpr size_t MAGIC_COLS = 12;

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

	void Log(const std::string& msg)
	{
		OutputDebugStringA(("[EnemyWeaponDatabase] " + msg + "\n").c_str());
	}

	// CSV を読み、データ行(空行・#コメント・ヘッダ行を除く)を列に分けて返す
	bool ReadRows(const std::string& path, size_t cols,
		std::vector<std::vector<std::string>>& rows)
	{
		std::ifstream ifs(path);
		if (!ifs)
		{
			Log("ファイルを開けません: " + path);
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

			std::vector<std::string> c;
			std::stringstream ss(line);
			std::string cell;
			while (std::getline(ss, cell, ','))
			{
				c.push_back(Trim(cell));
			}

			// ヘッダ行と、Excel が出す区切り用の空行(",,,,")は読み飛ばす
			if (c.empty() || c[0].empty() || c[0] == "name")
			{
				continue;
			}

			if (c.size() < cols)
			{
				Log(path + " " + std::to_string(lineNo) + "行目: 列数が不足しています("
					+ std::to_string(c.size()) + "/" + std::to_string(cols) + ")");
				continue;
			}
			rows.push_back(std::move(c));
		}
		return true;
	}

	std::map<std::string, WeaponData> LoadWeaponTable(void)
	{
		std::map<std::string, WeaponData> table;

		std::vector<std::vector<std::string>> rows;
		if (!ReadRows(WEAPON_CSV_PATH, WEAPON_COLS, rows))
		{
			return table;
		}

		for (const auto& c : rows)
		{
			try
			{
				WeaponData d;
				d.modelPath = c[1];
				d.handFrameName = c[2];
				d.localPos = VGet(std::stof(c[3]), std::stof(c[4]), std::stof(c[5]));
				d.localRot = VGet(std::stof(c[6]), std::stof(c[7]), std::stof(c[8]));
				d.scale = std::stof(c[9]);
				d.damage = std::stoi(c[10]);
				d.range = std::stof(c[11]);
				d.cooldownFrames = std::stoi(c[12]);
				d.attackFrames = std::stoi(c[13]);
				d.hitStartFrame = std::stoi(c[14]);
				d.hitEndFrame = std::stoi(c[15]);
				d.hitLength = std::stof(c[16]);
				d.hitRadius = std::stof(c[17]);
				table[c[0]] = d;
			}
			catch (const std::exception&)
			{
				Log("数値の読み取りに失敗: " + c[0]);
			}
		}
		return table;
	}

	std::map<std::string, MagicData> LoadMagicTable(void)
	{
		std::map<std::string, MagicData> table;

		std::vector<std::vector<std::string>> rows;
		if (!ReadRows(MAGIC_CSV_PATH, MAGIC_COLS, rows))
		{
			return table;
		}

		for (const auto& c : rows)
		{
			try
			{
				MagicData d;
				d.range = std::stof(c[1]);
				d.damage = std::stoi(c[2]);
				d.cooldownFrames = std::stoi(c[3]);
				d.castFrames = std::stoi(c[4]);
				d.fireFrame = std::stoi(c[5]);
				d.bulletSpeed = std::stof(c[6]);
				d.bulletRadius = std::stof(c[7]);
				d.bulletLife = std::stoi(c[8]);
				d.muzzleHeight = std::stof(c[9]);
				d.aimHeight = std::stof(c[10]);
				// 色は 0x8040ff のような16進表記(0x 付きなら基数を自動判定)
				d.color = static_cast<unsigned int>(std::stoul(c[11], nullptr, 0));
				table[c[0]] = d;
			}
			catch (const std::exception&)
			{
				Log("数値の読み取りに失敗: " + c[0]);
			}
		}
		return table;
	}
}

const WeaponData* FindWeaponData(const std::string& name)
{
	// 初回呼び出し時に1回だけ読み込む
	static const std::map<std::string, WeaponData> table = LoadWeaponTable();

	auto it = table.find(name);
	return it == table.end() ? nullptr : &it->second;
}

const MagicData* FindMagicData(const std::string& name)
{
	static const std::map<std::string, MagicData> table = LoadMagicTable();

	auto it = table.find(name);
	return it == table.end() ? nullptr : &it->second;
}

std::unique_ptr<EnemyAttackBase> CreateAttack(const std::string& name, int ownerModelId)
{
	if (const WeaponData* w = FindWeaponData(name))
	{
		return std::make_unique<EnemySword>(*w, ownerModelId);
	}
	if (const MagicData* m = FindMagicData(name))
	{
		return std::make_unique<EnemyMagic>(*m);
	}
	return nullptr;
}