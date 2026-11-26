#pragma once
#include <memory>
#include <string>
#include "EnemyAttackBase.h"
#include "EnemySword.h"
#include "EnemyMagic.h"

// 攻撃名 → 攻撃データ(CSV から読み込む)
// 初回アクセス時に自動で読み込まれる。見つからなければ nullptr を返す
//   近接武器: Data/CSV/EnemyWeapon.csv
//   魔法    : Data/CSV/EnemyMagic.csv
const WeaponData* FindWeaponData(const std::string& name);
const MagicData* FindMagicData(const std::string& name);

// 攻撃名から攻撃クラスを生成する(見つからなければ nullptr)
// 新しい種類の攻撃クラスを作ったら、EnemyWeaponDatabase.cpp の中に1行足す
std::unique_ptr<EnemyAttackBase> CreateAttack(const std::string& name, int ownerModelId);