#include <functional>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Manager/RunManager.h"
#include "../Object/SkyDome.h"
#include "EventScene.h"

// ---- イベント定義(ここに足すだけで増やせる) ----
struct EventScene::Choice
{
	std::string label;
	std::function<std::string(RunManager&)> effect;   // 結果テキストを返す
};

struct EventScene::Event
{
	std::string title;
	std::string text;
	std::vector<Choice> choices;
};

// 公開しない定義は静的関数にまとめる
namespace
{
	// 選択肢の表示位置(Draw と当たり判定で共通)
	constexpr int CHOICE_Y0 = 280;
	constexpr int CHOICE_STEP = 40;
	constexpr int CHOICE_W = 520;
	constexpr int CHOICE_PAD_Y = 8;

	int ChoiceLeft(void) { return Application::SCREEN_SIZE_X / 2 - 250; }
}

static std::vector<EventScene::Event> BuildEvents(RunManager::NODE_TYPE type)
{
	using E = EventScene::Event;
	using C = EventScene::Choice;
	std::vector<E> list;

	if (type == RunManager::NODE_TYPE::REST)
	{
		list.push_back(E{
			"野営地",
			"焚き火のそばで一息つける場所を見つけた。",
			{
				C{ "休む(最大HPの50%回復)", [](RunManager& r) {
					const int v = (r.GetHpMax() + 1) / 2;
					r.Heal(v);
					return "HPが " + std::to_string(v) + " 回復した。"; } },
				C{ "鍛える(最大HP+1)", [](RunManager& r) {
					r.AddHpMax(1);
					return std::string("最大HPが 1 増えた。"); } },
			} });
		return list;
	}

	list.push_back(E{
		"静かな泉",
		"澄んだ水をたたえた泉がある。",
		{
			C{ "水を飲む(HP30%回復)", [](RunManager& r) {
				const int v = (r.GetHpMax() * 3 + 9) / 10;
				r.Heal(v);
				return "HPが " + std::to_string(v) + " 回復した。"; } },
			C{ "立ち去る", [](RunManager&) {
				return std::string("何も起きなかった。"); } },
		} });

	list.push_back(E{
		"古い祭壇",
		"血のように赤い祭壇だ。力を求めるなら代償が要りそうだ。",
		{
			C{ "血を捧げる(HP-2、最大HP+2)", [](RunManager& r) {
				r.Damage(2);
				r.AddHpMax(2);
				return std::string("体に力がみなぎる……。最大HPが 2 増えた。"); } },
			C{ "立ち去る", [](RunManager&) {
				return std::string("祭壇に背を向けた。"); } },
		} });

	list.push_back(E{
		"怪しい薬瓶",
		"道端に薬瓶が落ちている。",
		{
			C{ "飲んでみる", [](RunManager& r) {
				if (r.Random(2) == 0)
				{
					r.Heal(r.GetHpMax());
					return std::string("体が軽い! HPが全回復した。");
				}
				r.Damage(2);
				return std::string("ひどい味だ……。HPが 2 減った。"); } },
			C{ "無視する", [](RunManager&) {
				return std::string("触らないことにした。"); } },
		} });

	return list;
}

EventScene::EventScene(void)
	: phase_(PHASE::SELECT), eventIdx_(0), cursor_(0),
	prevMouse_(0), prevMouseX_(0), prevMouseY_(0)
{
}

EventScene::~EventScene(void)
{
}

void EventScene::Init(void)
{
	spaceDomeTran_.pos = AsoUtility::VECTOR_ZERO;
	skyDome_ = std::make_unique<SkyDome>(spaceDomeTran_);
	skyDome_->Init();

	mainCamera.ChangeMode(Camera::MODE::FIXED_POINT);

	RunManager& run = RunManager::GetInstance();
	const auto events = BuildEvents(run.GetCurrentNode().type);
	eventIdx_ = run.Random(static_cast<int>(events.size()));
	cursor_ = 0;
	phase_ = PHASE::SELECT;

	// 前のシーンのクリックを持ち越さない
	prevMouse_ = GetMouseInput();
	GetMousePoint(&prevMouseX_, &prevMouseY_);
}

void EventScene::Update(void)
{
	skyDome_->Update();

	InputManager& ins = InputManager::GetInstance();
	RunManager& run = RunManager::GetInstance();

	// マウス状態
	int mx, my;
	GetMousePoint(&mx, &my);
	const int mouse = GetMouseInput();
	const bool click = (mouse & MOUSE_INPUT_LEFT) && !(prevMouse_ & MOUSE_INPUT_LEFT);
	const bool moved = (mx != prevMouseX_ || my != prevMouseY_);
	prevMouse_ = mouse;
	prevMouseX_ = mx;
	prevMouseY_ = my;

	if (phase_ == PHASE::SELECT)
	{
		const auto events = BuildEvents(run.GetCurrentNode().type);
		const auto& ev = events[eventIdx_];
		const int n = static_cast<int>(ev.choices.size());

		if (ins.IsTrgDown(KEY_INPUT_DOWN))
		{
			cursor_ = (cursor_ + 1) % n;
		}
		if (ins.IsTrgDown(KEY_INPUT_UP))
		{
			cursor_ = (cursor_ + n - 1) % n;
		}

		// マウスが乗っている選択肢
		int hit = -1;
		const int left = ChoiceLeft();
		for (int i = 0; i < n; ++i)
		{
			const int top = CHOICE_Y0 + i * CHOICE_STEP - CHOICE_PAD_Y;
			if (mx >= left - 10 && mx <= left + CHOICE_W
				&& my >= top && my < top + CHOICE_STEP)
			{
				hit = i;
				break;
			}
		}
		if (hit >= 0 && moved)
		{
			cursor_ = hit;
		}

		bool decide = ins.IsTrgDown(KEY_INPUT_SPACE);
		if (click && hit >= 0)
		{
			cursor_ = hit;
			decide = true;
		}

		if (decide)
		{
			resultText_ = ev.choices[cursor_].effect(run);
			phase_ = PHASE::RESULT;
		}
	}
	else
	{
		// 結果表示中は、Space か画面のどこをクリックしても進む
		if (ins.IsTrgDown(KEY_INPUT_SPACE) || click)
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::MAP);
		}
	}
}

void EventScene::Draw(void)
{
	skyDome_->Draw();

	RunManager& run = RunManager::GetInstance();
	const auto events = BuildEvents(run.GetCurrentNode().type);
	const auto& ev = events[eventIdx_];

	const int cx = ChoiceLeft();
	const unsigned int white = GetColor(255, 255, 255);

	DrawFormatString(20, 20, white, "HP %d / %d", run.GetHp(), run.GetHpMax());
	DrawString(cx, 150, ev.title.c_str(), GetColor(255, 220, 100));
	DrawString(cx, 190, ev.text.c_str(), white);

	if (phase_ == PHASE::SELECT)
	{
		for (size_t i = 0; i < ev.choices.size(); ++i)
		{
			const bool sel = (static_cast<int>(i) == cursor_);
			const int y = CHOICE_Y0 + static_cast<int>(i) * CHOICE_STEP;
			if (sel)
			{
				DrawBox(cx - 10, y - CHOICE_PAD_Y, cx + CHOICE_W,
					y - CHOICE_PAD_Y + CHOICE_STEP - 4, GetColor(0, 255, 255), false);
			}
			DrawFormatString(cx, y,
				sel ? GetColor(0, 255, 255) : GetColor(180, 180, 180),
				"%s %s", sel ? ">" : " ", ev.choices[i].label.c_str());
		}
		DrawString(20, Application::SCREEN_SIZE_Y - 30,
			"↑ ↓ / マウス : 選択    SPACE / クリック : 決定", GetColor(200, 200, 200));
	}
	else
	{
		DrawString(cx, 300, resultText_.c_str(), white);
		DrawString(20, Application::SCREEN_SIZE_Y - 30,
			"SPACE / クリック : マップへ", GetColor(200, 200, 200));
	}
}