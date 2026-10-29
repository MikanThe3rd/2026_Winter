#pragma once
#include <memory>
#include <string>
#include <vector>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class SkyDome;

// イベント/休憩ノードで表示する選択肢画面
class EventScene : public SceneBase
{
public:

	EventScene(void);
	~EventScene(void);

	// 選択肢とイベント(定義は EventScene.cpp)
	struct Choice;
	struct Event;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	enum class PHASE { SELECT, RESULT };

	Transform spaceDomeTran_;
	std::unique_ptr<SkyDome> skyDome_;

	PHASE phase_;
	int eventIdx_;
	int cursor_;
	std::string resultText_;

	// マウス(クリックの検出とホバー用)
	int prevMouse_;
	int prevMouseX_;
	int prevMouseY_;
};