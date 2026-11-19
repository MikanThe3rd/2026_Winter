#pragma once
#include "EnemyBase.h"
class Sister : public EnemyBase
{
	public:

	static constexpr double ROT_RATE = 0.1;

	// コンストラクタ
	Sister(void);

	// デストラクタ
	~Sister(void);

	void Init(void) override;
	void Draw(void) override;
	void InitAnimation(void) override;
};

