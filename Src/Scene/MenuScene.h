#pragma once
#include "SceneBase.h"

class MenuScene : public SceneBase
{
public:

	MenuScene(void);
	~MenuScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	int selectNo_;
};

