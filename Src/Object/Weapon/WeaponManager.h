#pragma once
#include <memory>
#include <vector>
#include "../Common/Transform.h"

class WeaponBase;

// 武器の生成・切り替え・更新・描画をまとめて管理する
class WeaponManager
{
public:

	enum class TYPE
	{
		SWORD,
		CLAW,
		MAX
	};

	WeaponManager(void);
	~WeaponManager(void);

	// 全種類の武器を生成して初期化する
	void Init(TYPE first = TYPE::SWORD);

	void Update(const Transform& playerTransform, bool isAttacking, float attackTime, bool isSpecial = false);
	void Draw(void);

	bool Switch(TYPE type);
	bool SwitchNext(void);
	bool SwitchPrev(void);

	void SetDrawAll(bool drawAll) { drawAll_ = drawAll; }
	bool IsDrawAll(void) const { return drawAll_; }

	// 現在装備中の武器
	WeaponBase& GetActive(void);
	const WeaponBase& GetActive(void) const;
	TYPE GetActiveType(void) const { return static_cast<TYPE>(active_); }

private:

	std::vector<std::unique_ptr<WeaponBase>> weapons_;

	int active_;

	bool drawAll_;
	bool isAttacking_;

	std::unique_ptr<WeaponBase> Create(TYPE type) const;
};