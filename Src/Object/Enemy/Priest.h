#pragma once
#include"EnemyBase.h"

class Priest :public EnemyBase
{
public:

	Priest(void);
	~Priest(void);
	void Init(void) override;
	void Draw(void) override;
	void InitAnimation(void) override;
};

