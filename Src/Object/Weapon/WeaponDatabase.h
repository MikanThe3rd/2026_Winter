#pragma once
#include <map>
#include <string>
#include "WeaponBase.h"

class WeaponDatabase
{
public:

	// 武器設定ファイル(実行時のカレントディレクトリからの相対パス)
	static constexpr const char* FILE_PATH = "Data/Weapon/Weapon.csv";

	static WeaponDatabase& GetInstance(void);

	// CSV を読み込む(再度呼ぶと読み直す。調整中の確認に便利)
	void Load(void);

	// 名前からパラメーターを取得(未読み込みなら自動で読み込む)
	// 見つからなければ WeaponParam のデフォルト値を返す
	const WeaponParam& Get(const std::string& name);

private:

	WeaponDatabase(void) = default;

	bool loaded_ = false;
	std::map<std::string, WeaponParam> table_;
	WeaponParam default_;
};