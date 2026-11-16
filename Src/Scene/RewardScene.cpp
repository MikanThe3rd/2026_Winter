#include <DxLib.h>
#include "../Application.h"
#include "../Manager/Camera.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "RewardScene.h"

RewardScene::RewardScene(bool elite)
	: elite_(elite), cursor_(0), timer_(0.0f),
	prevMouse_(0), prevMouseX_(0), prevMouseY_(0)
{
}

RewardScene::~RewardScene(void)
{
}

void RewardScene::Init(void)
{
	rewards_ = RunManager::GetInstance().PickRewards(elite_, 3);
	cursor_ = 0;
	timer_ = 0.0f;

	// 戦闘中のクリックを持ち越さない
	prevMouse_ = GetMouseInput();
	GetMousePoint(&prevMouseX_, &prevMouseY_);

	mainCamera.SetInputLock(true);
}

void RewardScene::GetCardPos(int i, int& x, int& y) const
{
	const int n = static_cast<int>(rewards_.size());
	const int total = n * CARD_W + (n - 1) * CARD_GAP;
	x = (Application::SCREEN_SIZE_X - total) / 2 + i * (CARD_W + CARD_GAP);
	y = (Application::SCREEN_SIZE_Y - CARD_H) / 2;
}

int RewardScene::HitCard(int mx, int my) const
{
	for (int i = 0; i < static_cast<int>(rewards_.size()); ++i)
	{
		int x, y;
		GetCardPos(i, x, y);
		if (mx >= x && mx <= x + CARD_W && my >= y && my <= y + CARD_H)
		{
			return i;
		}
	}
	return -1;
}

void RewardScene::Update(void)
{
	timer_ += SceneManager::GetInstance().GetDeltaTime();

	int mx, my;
	GetMousePoint(&mx, &my);
	const int mouse = GetMouseInput();
	const bool click = (mouse & MOUSE_INPUT_LEFT) && !(prevMouse_ & MOUSE_INPUT_LEFT);
	const bool moved = (mx != prevMouseX_ || my != prevMouseY_);
	prevMouse_ = mouse;
	prevMouseX_ = mx;
	prevMouseY_ = my;

	// 表示直後は入力を受け付けない
	if (timer_ < INPUT_DELAY || rewards_.empty())
	{
		return;
	}

	InputManager& ins = InputManager::GetInstance();
	const int n = static_cast<int>(rewards_.size());
	constexpr auto PAD = InputManager::JOYPAD_NO::PAD1;
	using DIR = InputManager::JOYPAD_DIR;

	if (ins.IsTrgDown(KEY_INPUT_RIGHT) || ins.IsPadDirTrgDown(PAD, DIR::RIGHT))
	{
		cursor_ = (cursor_ + 1) % n;
	}
	if (ins.IsTrgDown(KEY_INPUT_LEFT) || ins.IsPadDirTrgDown(PAD, DIR::LEFT))
	{
		cursor_ = (cursor_ + n - 1) % n;
	}

	// マウスは動かしたときだけホバーで追従
	const int hit = HitCard(mx, my);
	if (hit >= 0 && moved)
	{
		cursor_ = hit;
	}

	bool decide = ins.IsTrgDown(KEY_INPUT_SPACE)
		|| ins.IsPadBtnTrgDown(PAD, InputManager::JOYPAD_BTN::DOWN);
	if (click && hit >= 0)
	{
		cursor_ = hit;
		decide = true;
	}

	if (decide)
	{
		RunManager::GetInstance().ApplyReward(rewards_[cursor_]);
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::MAP);
	}
}

void RewardScene::Draw(void)
{
	const int sw = Application::SCREEN_SIZE_X;
	const int sh = Application::SCREEN_SIZE_Y;

	// 暗幕(下の GameScene が透けて見える)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
	DrawBox(0, 0, sw, sh, GetColor(0, 0, 0), true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	const int oldSize = GetFontSize();

	// 見出し
	SetFontSize(32);
	const char* title = elite_ ? "強敵撃破! 強力な強化を選べ" : "戦闘勝利! 強化を選べ";
	DrawString((sw - GetDrawStringWidth(title, -1)) / 2, sh / 2 - CARD_H / 2 - 70,
		title, elite_ ? GetColor(255, 210, 80) : GetColor(255, 255, 255));

	for (int i = 0; i < static_cast<int>(rewards_.size()); ++i)
	{
		int x, y;
		GetCardPos(i, x, y);
		const auto& r = rewards_[i];
		const bool sel = (i == cursor_);

		const unsigned int bg = r.elite ? GetColor(70, 50, 15) : GetColor(30, 40, 60);
		const unsigned int edge = sel ? GetColor(0, 255, 255)
			: (r.elite ? GetColor(230, 170, 40) : GetColor(150, 150, 150));

		DrawBox(x, y, x + CARD_W, y + CARD_H, bg, true);
		DrawBox(x, y, x + CARD_W, y + CARD_H, edge, false);
		if (sel)
		{
			DrawBox(x - 3, y - 3, x + CARD_W + 3, y + CARD_H + 3, edge, false);
		}

		SetFontSize(28);
		DrawString(x + (CARD_W - GetDrawStringWidth(r.name.c_str(), -1)) / 2, y + 40,
			r.name.c_str(), GetColor(255, 255, 255));

		SetFontSize(22);
		DrawString(x + (CARD_W - GetDrawStringWidth(r.desc.c_str(), -1)) / 2, y + 150,
			r.desc.c_str(), GetColor(220, 220, 220));
	}

	SetFontSize(oldSize);
}