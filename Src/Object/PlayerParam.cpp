#include <string>
#include <vector>
#include <DxLib.h>
#include "../Utility/CsvUtility.h"
#include "PlayerParam.h"

PlayerParam PlayerParam::Load(void)
{
	PlayerParam p;

	std::vector<std::vector<std::string>> rows;
	if (!CsvUtility::Load(FILE_PATH, rows))
	{
		OutputDebugStringA("[PlayerParam] ステータスファイルを開けませんでした(デフォルト値を使用)\n");
		return p;
	}

	// 形式: 項目名, 値(1行に1項目)
	for (const auto& c : rows)
	{
		if (c.size() < 2)
		{
			continue;
		}

		const std::string& key = c[0];
		if (key == "hp")
		{
			p.hp = CsvUtility::ToInt(c, 1, p.hp);
		}
		else if (key == "speedMove")
		{
			p.speedMove = CsvUtility::ToFloat(c, 1, p.speedMove);
		}
		else if (key == "speedRun")
		{
			p.speedRun = CsvUtility::ToFloat(c, 1, p.speedRun);
		}
		else if (key == "timeRot")
		{
			p.timeRot = CsvUtility::ToFloat(c, 1, p.timeRot);
		}
		else if (key == "jumpPow")
		{
			p.jumpPow = CsvUtility::ToFloat(c, 1, p.jumpPow);
		}
		else
		{
			OutputDebugStringA(("[PlayerParam] 未知の項目: " + key + "\n").c_str());
		}
	}

	// 不正な値の補正
	if (p.timeRot < 0.01f)
	{
		p.timeRot = 0.01f;
	}
	if (p.hp < 1)
	{
		p.hp = 1;
	}

	return p;
}
