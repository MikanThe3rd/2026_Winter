#pragma once
#include "EnemyBase.h"
class Sister : public EnemyBase
{
public:

	// コンストラクタ
	Sister(void);

	// デストラクタ
	~Sister(void);

	void Init(void) override;
	void Draw(void) override;
	void InitAnimation(void) override;

protected:

	void ChangeStateEngage(void) override;
	void ChangeStateStun(void) override;
	void ChangeStateAttack(void) override;

	void UpdateSearch(void) override;
	void UpdateTracking(void) override;
	void UpdateEngage(void) override;
	void UpdateStun(void) override;
	void UpdateAttack(void) override;
};

