#pragma once
#include <map>
#include <string>
#include "EnemySword.h"

// 武器名 → 武器データ(簡易版。増えてきたら CSV 読み込みに切り替える)
// 各値は WeaponData の宣言順:
//  modelPath, handFrameName, localPos, localRot, scale,
//  damage, range, cooldownFrames, attackFrames,
//  hitStartFrame, hitEndFrame, hitLength, hitRadius
inline const WeaponData* FindWeaponData(const std::string& name)
{
	static const std::map<std::string, WeaponData> table =
	{
		{ "Sword",
			{
				"Data/Model/Weapon/Sword.mv1",   
				"mixamorig:RightHand",           
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				1.0f,
				1,        // damage
				120.0f,   // range
				90,       // cooldownFrames
				180,       // attackFrames
				40,       // hitStartFrame
				180,       // hitEndFrame
				100.0f,   // hitLength
				25.0f     // hitRadius
			}
		},
	};

	auto it = table.find(name);
	return it == table.end() ? nullptr : &it->second;
}