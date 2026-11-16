#pragma once
#include <vector>
#include "SceneBase.h"
#include "../Manager/RunManager.h"

// í“¬Œã‚Ì‹­‰»‚ğ‘I‚Ô‰æ–Ê(PushScene ‚Åd‚Ë‚Äg‚¤)
class RewardScene : public SceneBase
{
public:

	// elite: ‹­“G•ñV(‹­‰»”Å)‚É‚·‚é‚©
	explicit RewardScene(bool elite);
	~RewardScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	static constexpr float INPUT_DELAY = 0.6f;   // •\¦’¼Œã‚ÌŒë‘€ì–h~(•b)
	static constexpr int CARD_W = 260;
	static constexpr int CARD_H = 300;
	static constexpr int CARD_GAP = 40;

	bool elite_;
	std::vector<RunManager::Reward> rewards_;
	int cursor_;
	float timer_;

	int prevMouse_;
	int prevMouseX_;
	int prevMouseY_;

	void GetCardPos(int i, int& x, int& y) const;
	int HitCard(int mx, int my) const;
};