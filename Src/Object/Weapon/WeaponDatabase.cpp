#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "../../Utility/CSVUtility.h"
#include "WeaponDatabase.h"

WeaponDatabase& WeaponDatabase::GetInstance(void)
{
	static WeaponDatabase instance;
	return instance;
}

void WeaponDatabase::Load(void)
{
	loaded_ = true;
	table_.clear();

	std::vector<std::vector<std::string>> rows;
	if (!CsvUtility::Load(FILE_PATH, rows))
	{
		OutputDebugStringA("[WeaponDatabase] 武器設定ファイルを開けませんでした\n");
		return;
	}

	const WeaponParam def;
	for (const auto& c : rows)
	{
		if (c.empty() || c[0].empty())
		{
			continue;
		}

		WeaponParam p;
		p.damage = CsvUtility::ToInt(c, 1, def.damage);
		p.hitStart = CsvUtility::ToFloat(c, 2, def.hitStart);
		p.hitEnd = CsvUtility::ToFloat(c, 3, def.hitEnd);

		float s = CsvUtility::ToFloat(c, 4, 1.0f);
		p.scale = VGet(s, s, s);

		p.capTop = VGet(
			CsvUtility::ToFloat(c, 5, def.capTop.x),
			CsvUtility::ToFloat(c, 6, def.capTop.y),
			CsvUtility::ToFloat(c, 7, def.capTop.z));
		p.capDown = VGet(
			CsvUtility::ToFloat(c, 8, def.capDown.x),
			CsvUtility::ToFloat(c, 9, def.capDown.y),
			CsvUtility::ToFloat(c, 10, def.capDown.z));
		p.capRadius = CsvUtility::ToFloat(c, 11, def.capRadius);

		p.offset.localPos = VGet(
			CsvUtility::ToFloat(c, 12, 0.0f),
			CsvUtility::ToFloat(c, 13, 0.0f),
			CsvUtility::ToFloat(c, 14, 0.0f));

		// CSV は度、内部はラジアン
		p.offset.rotEuler = VGet(
			AsoUtility::Deg2RadF(CsvUtility::ToFloat(c, 15, 0.0f)),
			AsoUtility::Deg2RadF(CsvUtility::ToFloat(c, 16, 0.0f)),
			AsoUtility::Deg2RadF(CsvUtility::ToFloat(c, 17, 0.0f)));

		// 特殊攻撃の判定窓(省略時は WeaponParam のデフォルト値)
		p.specialHitStart = CsvUtility::ToFloat(c, 18, def.specialHitStart);
		p.specialHitEnd = CsvUtility::ToFloat(c, 19, def.specialHitEnd);

		table_[c[0]] = p;
	}
}

const WeaponParam& WeaponDatabase::Get(const std::string& name)
{
	if (!loaded_)
	{
		Load();
	}

	auto it = table_.find(name);
	if (it == table_.end())
	{
		OutputDebugStringA(("[WeaponDatabase] 未定義の武器: " + name + "\n").c_str());
		return default_;
	}
	return it->second;
}