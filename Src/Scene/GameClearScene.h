#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class SceneManager;
class SkyDome;

class GameClearScene :public SceneBase
{
public:
	GameClearScene(void);
	~GameClearScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
	

	// スカイドーム用
	Transform spaceDomeTran_;

	// スカイドーム(背景)
	std::unique_ptr<SkyDome> skyDome_;

	
};