#pragma once
#include <memory>
#include <DxLib.h>
#include "EnemyBase.h"

class Knight : public EnemyBase
{
public:

	static constexpr double ROT_RATE = 0.1;

	// コンストラクタ
	Knight(void);

	// デストラクタ
	~Knight(void);

	void Init(void) override;
	void Draw(void) override;
	void InitAnimation(void) override;

protected:

	void UpdateSearch(void) override;
	void UpdateTracking(void) override;

};