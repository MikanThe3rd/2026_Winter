#pragma once
#include <memory>
#include <DxLib.h>
#include "EnemyBase.h"

class Knight : public EnemyBase
{
public:

	

	// コンストラクタ
	Knight(void);

	// デストラクタ
	~Knight(void);

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