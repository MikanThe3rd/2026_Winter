#pragma once
#include "WeaponBase.h"

class Claw :public WeaponBase
{
public:

	Claw(void);
	virtual ~Claw(void);

	void Init(void) override;
	void Update(const Transform& playerTransform) override;
	void Draw(void) override;

	// çXêVä÷åW
	void UpdateTransform(const Transform& playerTransform) override;
};