#pragma once
#include <string>
#include <vector>

namespace CsvUtility
{
	// CSV を読み込み、行ごとの列リストにする
	// 空行と、# で始まる行はスキップ。列は前後の空白を除去する
	bool Load(const std::string& path,
		std::vector<std::vector<std::string>>& rows);

	// idx 列目を数値に変換(列がない・空欄・変換失敗ならデフォルト値)
	float ToFloat(const std::vector<std::string>& cols, size_t idx, float def);
	int ToInt(const std::vector<std::string>& cols, size_t idx, int def);
}