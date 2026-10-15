#pragma once
#include <memory>
#include <vector>
#include "../Common/Transform.h"

class WeaponBase;

// 武器の生成・切り替え・更新・描画をまとめて管理する
class WeaponManager
{
public:

	// 武器の種類(武器を増やすときはここに追加し、Create() に case を足す)
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

	// attackTime は Player::GetAttackTime() の値
	void Update(const Transform& playerTransform, bool isAttacking, float attackTime);
	void Draw(void);

	// 切り替え。成功したら true
	// (攻撃中は切り替えない / すでにその武器なら何もしない)
	bool Switch(TYPE type);
	bool SwitchNext(void);
	bool SwitchPrev(void);

	// true にすると、装備中でない武器も含めて全種類を描画する
	// (二種類同時に持たせて見たいとき用。当たり判定は装備中の武器だけ)
	void SetDrawAll(bool drawAll) { drawAll_ = drawAll; }
	bool IsDrawAll(void) const { return drawAll_; }

	// 現在装備中の武器(当たり判定に使う)
	WeaponBase& GetActive(void);
	const WeaponBase& GetActive(void) const;
	TYPE GetActiveType(void) const { return static_cast<TYPE>(active_); }

private:

	std::vector<std::unique_ptr<WeaponBase>> weapons_;

	// 装備中の武器の添字(TYPE と対応)
	int active_;

	bool drawAll_;
	bool isAttacking_;

	// 種類から武器を生成する
	std::unique_ptr<WeaponBase> Create(TYPE type) const;
};