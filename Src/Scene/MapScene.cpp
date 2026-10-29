#include <cmath>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Manager/RunManager.h"
#include "../Manager/ResourceManager.h"
#include "../Object/SkyDome.h"
#include "MapScene.h"

namespace
{
	constexpr int NODE_RADIUS = 36;
	constexpr float ICON_SCALE = 0.8f;     
	constexpr int FLOOR_GAP_X = 240;       // 階層ごとの横間隔(画面に収まらなくてよい)
	constexpr int NODE_GAP_Y = 130;        // 同じ階層のノード同士の縦間隔
	constexpr float ANCHOR_RATE = 0.25f;   // 現在地を画面の左から何割に置くか
	constexpr float SCROLL_SPEED = 6.0f;   // スクロールの追従の速さ

	// 種別ごとの色(rate で暗くできる)
	unsigned int NodeColor(RunManager::NODE_TYPE t, float rate = 1.0f)
	{
		int r = 255, g = 255, b = 255;
		switch (t)
		{
		case RunManager::NODE_TYPE::BATTLE: r = 200; g = 70;  b = 70;  break;
		case RunManager::NODE_TYPE::ELITE:  r = 160; g = 40;  b = 160; break;
		case RunManager::NODE_TYPE::EVENT:  r = 70;  g = 130; b = 220; break;
		case RunManager::NODE_TYPE::REST:   r = 70;  g = 190; b = 100; break;
		case RunManager::NODE_TYPE::BOSS:   r = 230; g = 160; b = 30;  break;
		}
		return GetColor(static_cast<int>(r * rate),
			static_cast<int>(g * rate), static_cast<int>(b * rate));
	}

	// ノード種別に対応するアイコン画像のハンドル(休憩は未用意のため -1)
	int LoadNodeIcon(ResourceManager& res, RunManager::NODE_TYPE t)
	{
		using SRC = ResourceManager::SRC;
		switch (t)
		{
		case RunManager::NODE_TYPE::BATTLE: return res.Load(SRC::BATTLE_ICON_NORMAL).handleId_;
		case RunManager::NODE_TYPE::ELITE:  return res.Load(SRC::BATTLE_ICON_ELITE).handleId_;
		case RunManager::NODE_TYPE::EVENT:  return res.Load(SRC::EVENT_ICON).handleId_;
		case RunManager::NODE_TYPE::BOSS:   return res.Load(SRC::BATTLE_ICON_BOSS).handleId_;
		default: return -1;
		}
	}

	const char* NodeMark(RunManager::NODE_TYPE t)
	{
		switch (t)
		{
		case RunManager::NODE_TYPE::BATTLE: return "戦";
		case RunManager::NODE_TYPE::ELITE:  return "強";
		case RunManager::NODE_TYPE::EVENT:  return "?";
		case RunManager::NODE_TYPE::REST:   return "休";
		case RunManager::NODE_TYPE::BOSS:   return "王";
		}
		return "";
	}

	const char* NodeName(RunManager::NODE_TYPE t)
	{
		switch (t)
		{
		case RunManager::NODE_TYPE::BATTLE: return "戦闘";
		case RunManager::NODE_TYPE::ELITE:  return "強敵";
		case RunManager::NODE_TYPE::EVENT:  return "イベント";
		case RunManager::NODE_TYPE::REST:   return "休憩";
		case RunManager::NODE_TYPE::BOSS:   return "ボス";
		}
		return "";
	}
}

MapScene::MapScene(void)
	: cursor_(0), selectFloor_(0), scrollX_(0.0f),
	prevMouse_(0), prevMouseX_(0), prevMouseY_(0)
{
	for (int& h : imgIcon_)
	{
		h = -1;
	}
}

MapScene::~MapScene(void)
{
	// 画像は ResourceManager が管理する(シーン切替時に Release される)
}

float MapScene::CalcScroll(int focusFloor) const
{
	return static_cast<float>(focusFloor * FLOOR_GAP_X)
		- Application::SCREEN_SIZE_X * ANCHOR_RATE;
}

void MapScene::Init(void)
{
	spaceDomeTran_.pos = AsoUtility::VECTOR_ZERO;
	skyDome_ = std::make_unique<SkyDome>(spaceDomeTran_);
	skyDome_->Init();

	// ノードのアイコン画像
	for (int i = 0; i < ICON_NUM; ++i)
	{
		imgIcon_[i] = LoadNodeIcon(resMng_, static_cast<RunManager::NODE_TYPE>(i));
	}

	mainCamera.ChangeMode(Camera::MODE::FIXED_POINT);

	RunManager& run = RunManager::GetInstance();
	selectable_ = run.GetSelectableIndices();
	selectFloor_ = run.GetCurrentFloor() + 1;
	cursor_ = 0;

	// 前のシーンのクリックを持ち越さない
	prevMouse_ = GetMouseInput();
	GetMousePoint(&prevMouseX_, &prevMouseY_);

	// 現在地の階層を基準にする。1つ前の位置から滑り込ませて「進んだ感」を出す
	const int focus = (selectFloor_ - 1 > 0) ? selectFloor_ - 1 : 0;
	const int prev = (focus - 1 > 0) ? focus - 1 : 0;
	scrollX_ = CalcScroll(selectFloor_ >= 2 ? prev : focus);
}

void MapScene::Update(void)
{
	skyDome_->Update();

	// 現在地に向かってなめらかにスクロール
	const int focus = (selectFloor_ - 1 > 0) ? selectFloor_ - 1 : 0;
	const float dt = SceneManager::GetInstance().GetDeltaTime();
	const float t = 1.0f - std::exp(-SCROLL_SPEED * dt);
	scrollX_ += (CalcScroll(focus) - scrollX_) * t;

	// マウス状態
	int mx, my;
	GetMousePoint(&mx, &my);
	const int mouse = GetMouseInput();
	const bool click = (mouse & MOUSE_INPUT_LEFT) && !(prevMouse_ & MOUSE_INPUT_LEFT);
	const bool moved = (mx != prevMouseX_ || my != prevMouseY_);
	prevMouse_ = mouse;
	prevMouseX_ = mx;
	prevMouseY_ = my;

	if (selectable_.empty())
	{
		return;
	}

	InputManager& ins = InputManager::GetInstance();
	const int n = static_cast<int>(selectable_.size());

	// 選択肢は縦に並ぶので、↑↓で選ぶ(上のノードほど index が小さい)
	if (ins.IsTrgDown(KEY_INPUT_DOWN))
	{
		cursor_ = (cursor_ + 1) % n;
	}
	if (ins.IsTrgDown(KEY_INPUT_UP))
	{
		cursor_ = (cursor_ + n - 1) % n;
	}

	// マウス: 動かしたときだけホバーでカーソルを追従(キー操作を邪魔しない)
	const int hit = HitTest(mx, my);
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
		Decide();
	}
}

int MapScene::HitTest(int mx, int my) const
{
	const int r = NODE_RADIUS + 10;
	for (size_t i = 0; i < selectable_.size(); ++i)
	{
		int x, y;
		GetNodePos(selectFloor_, selectable_[i], x, y);
		const int dx = mx - x;
		const int dy = my - y;
		if (dx * dx + dy * dy <= r * r)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

void MapScene::Decide(void)
{
	RunManager& run = RunManager::GetInstance();
	run.SelectNode(selectable_[cursor_]);

	switch (run.GetCurrentNode().type)
	{
	case RunManager::NODE_TYPE::BATTLE:
	case RunManager::NODE_TYPE::ELITE:
	case RunManager::NODE_TYPE::BOSS:
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		break;
	case RunManager::NODE_TYPE::EVENT:
	case RunManager::NODE_TYPE::REST:
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::EVENT);
		break;
	}
}

void MapScene::GetNodePos(int floor, int index, int& x, int& y) const
{
	const RunManager& run = RunManager::GetInstance();
	const int w = run.GetFloorWidth(floor);

	// 左から右へ進行。画面幅に収めず、スクロール量だけずらす
	x = static_cast<int>(floor * FLOOR_GAP_X - scrollX_);
	y = Application::SCREEN_SIZE_Y / 2
		+ static_cast<int>((index - (w - 1) * 0.5f) * NODE_GAP_Y);
}

void MapScene::Draw(void)
{
	skyDome_->Draw();

	const RunManager& run = RunManager::GetInstance();
	const auto& map = run.GetMap();
	const int curF = run.GetCurrentFloor();
	const int curI = run.GetCurrentIndex();
	const int screenW = Application::SCREEN_SIZE_X;

	// 画面外の階層は描かない
	auto isVisible = [&](int floor)
		{
			int x, y;
			GetNodePos(floor, 0, x, y);
			return x > -NODE_RADIUS * 3 && x < screenW + NODE_RADIUS * 3;
		};

	// 接続線
	for (const auto& floor : map)
	{
		for (const auto& n : floor)
		{
			if (n.next.empty()) continue;
			if (!isVisible(n.floor) && !isVisible(n.floor + 1)) continue;

			int x1, y1;
			GetNodePos(n.floor, n.index, x1, y1);
			const bool fromCurrent = (n.floor == curF && n.index == curI);
			const bool onPath = (run.GetPathIndex(n.floor) == n.index);

			for (int k : n.next)
			{
				int x2, y2;
				GetNodePos(n.floor + 1, k, x2, y2);

				unsigned int col = GetColor(90, 90, 90);
				int width = 1;
				if (fromCurrent)
				{
					col = GetColor(255, 255, 255);
					width = 3;
				}
				else if (onPath && run.GetPathIndex(n.floor + 1) == k)
				{
					col = GetColor(255, 220, 100);   // 辿ってきた道
					width = 3;
				}
				DrawLine(x1, y1, x2, y2, col, width);
			}
		}
	}

	// ノード
	for (const auto& floor : map)
	{
		if (!isVisible(floor[0].floor)) continue;

		for (const auto& n : floor)
		{
			int x, y;
			GetNodePos(n.floor, n.index, x, y);

			// 通過済みで選ばなかったノードは暗くする
			const bool passedUnused = (n.floor <= curF
				&& run.GetPathIndex(n.floor) != n.index);
			const float rate = passedUnused ? 0.3f : 1.0f;

			const int icon = imgIcon_[static_cast<int>(n.type)];
			if (icon != -1)
			{
				// 画像アイコン(通過済みで選ばなかったものは暗く)
				const int b = static_cast<int>(255 * rate);
				SetDrawBright(b, b, b);
				DrawRotaGraph(x, y, ICON_SCALE, 0.0, icon, true);
				SetDrawBright(255, 255, 255);
			}
			else
			{
				// 画像がない種別(休憩など)は従来の円で描く
				DrawCircle(x, y, NODE_RADIUS, NodeColor(n.type, rate), true);
				DrawCircle(x, y, NODE_RADIUS, GetColor(20, 20, 20), false, 2);
				DrawString(x - 8, y - 8, NodeMark(n.type),
					passedUnused ? GetColor(120, 120, 120) : GetColor(255, 255, 255));
			}

			// 現在地 / 通過済み
			if (n.floor == curF && n.index == curI)
			{
				DrawCircle(x, y, NODE_RADIUS + 5, GetColor(255, 255, 0), false, 3);
			}
			else if (n.floor < curF && !passedUnused)
			{
				DrawCircle(x, y, NODE_RADIUS + 3, GetColor(180, 160, 80), false, 2);
			}
		}
	}

	// 選択可能ノードの強調とカーソル
	for (size_t i = 0; i < selectable_.size(); ++i)
	{
		int x, y;
		GetNodePos(selectFloor_, selectable_[i], x, y);
		DrawCircle(x, y, NODE_RADIUS + 5, GetColor(255, 255, 255), false, 2);

		if (static_cast<int>(i) == cursor_)
		{
			DrawCircle(x, y, NODE_RADIUS + 10, GetColor(0, 255, 255), false, 4);

			const auto& node = map[selectFloor_][selectable_[i]];
			DrawFormatString(screenW / 2 - 60, 30,
				GetColor(255, 255, 255), "次の行き先: %s", NodeName(node.type));
		}
	}

	DrawFormatString(20, 20, GetColor(255, 255, 255),
		"HP %d / %d", run.GetHp(), run.GetHpMax());
	DrawFormatString(screenW - 140, 20, GetColor(255, 255, 255),
		"階層 %d / %d", (curF + 1 > 0 ? curF + 1 : 0), RunManager::FLOOR_NUM);
	DrawString(20, Application::SCREEN_SIZE_Y - 30,
		"↑ ↓ / マウス : 選択    SPACE / クリック : 決定", GetColor(200, 200, 200));
}