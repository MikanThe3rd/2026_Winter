#pragma once
#include "WeaponBase.h"

class Sword : public WeaponBase
{
public:

	// コンストラクタ
	Sword(void);

	// デストラクタ
	~Sword(void);

	void Init(void) override;
	void Update(const Transform& playerTransform) override;
	void Draw(void) override;

	// 更新関係
	void UpdateTransform(const Transform& playerTransform) override;

};
