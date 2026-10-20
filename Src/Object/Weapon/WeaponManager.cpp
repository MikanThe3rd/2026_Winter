#include "WeaponBase.h"
#include "Sword.h"
#include "Claw.h"
#include "WeaponManager.h"

WeaponManager::WeaponManager(void)
	:
	active_(0),
	drawAll_(false),
	isAttacking_(false)
{
}

WeaponManager::~WeaponManager(void)
{
}

std::unique_ptr<WeaponBase> WeaponManager::Create(TYPE type) const
{
	switch (type)
	{
	case TYPE::SWORD:
		return std::make_unique<Sword>();
	case TYPE::CLAW:
		return std::make_unique<Claw>();
	default:
		return nullptr;
	}
}

void WeaponManager::Init(TYPE first)
{
	weapons_.clear();

	// ‘Sí—Ş‚ğ¶¬(TYPE ‚Ì•À‚Ñ‡ = “Yš)
	for (int i = 0; i < static_cast<int>(TYPE::MAX); i++)
	{
		auto w = Create(static_cast<TYPE>(i));
		w->Init();
		weapons_.push_back(std::move(w));
	}

	active_ = static_cast<int>(first);
}

void WeaponManager::Update(const Transform& playerTransform, bool isAttacking, float attackTime, bool isSpecial)
{
	isAttacking_ = isAttacking;

	for (int i = 0; i < static_cast<int>(weapons_.size()); i++)
	{
		const bool isActive = (i == active_);

		// •\¦‚³‚ê‚é•Ší‚¾‚¯ˆÊ’u‚ğXV‚·‚é
		if (isActive || drawAll_)
		{
			weapons_[i]->Update(playerTransform);
		}

		// ‘•”õ’†‚Ì•ŠíˆÈŠO‚Í”»’è‚ğí‚É–³Œø‚É‚·‚é
		weapons_[i]->UpdateHit(isActive && isAttacking,
			isActive ? attackTime : 0.0f,
			isActive && isSpecial);
	}
}

void WeaponManager::Draw(void)
{
	for (int i = 0; i < static_cast<int>(weapons_.size()); i++)
	{
		if (i == active_ || drawAll_)
		{
			weapons_[i]->Draw();
		}
	}
}

bool WeaponManager::Switch(TYPE type)
{
	const int next = static_cast<int>(type);
	if (next < 0 || next >= static_cast<int>(weapons_.size()))
	{
		return false;
	}

	// UŒ‚’†‚ÌØ‚è‘Ö‚¦‚Í‹Ö~(”»’è‚Ì—LŒø/–³Œø‚ª“r’†‚ÅØ‚ê‚é‚Ì‚ğ–h‚®)
	if (next == active_ || isAttacking_)
	{
		return false;
	}

	active_ = next;
	return true;
}

bool WeaponManager::SwitchNext(void)
{
	const int n = static_cast<int>(weapons_.size());
	return Switch(static_cast<TYPE>((active_ + 1) % n));
}

bool WeaponManager::SwitchPrev(void)
{
	const int n = static_cast<int>(weapons_.size());
	return Switch(static_cast<TYPE>((active_ + n - 1) % n));
}

WeaponBase& WeaponManager::GetActive(void)
{
	return *weapons_[active_];
}

const WeaponBase& WeaponManager::GetActive(void) const
{
	return *weapons_[active_];
}