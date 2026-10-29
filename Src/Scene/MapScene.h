#pragma once
#include <memory>
#include <vector>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class SkyDome;

// 行き先(ノード)を選ぶマップ画面
// 左から右へ横スクロールしながら進む
class MapScene : public SceneBase
{
public:

	MapScene(void);
	~MapScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

private:

	// 背景
	Transform spaceDomeTran_;
	std::unique_ptr<SkyDome> skyDome_;

	// 選択可能ノード(index)とカーソル位置
	std::vector<int> selectable_;
	int cursor_;

	// 選択肢が並んでいる階層(決定後に現在地が進んでも、描画がずれないよう保持)
	int selectFloor_;

	// ノード種別ごとのアイコン画像(RunManager::NODE_TYPE の順。未設定は -1)
	static constexpr int ICON_NUM = 5;
	int imgIcon_[ICON_NUM];

	// マウス(クリックの検出とホバー用)
	int prevMouse_;
	int prevMouseX_;
	int prevMouseY_;

	// マウス座標にある選択肢(selectable_ の添字)。なければ -1
	int HitTest(int mx, int my) const;

	// 選択中のノードに決定して遷移する
	void Decide(void);

	// 横スクロール量(ワールド座標 → 画面座標の差)
	float scrollX_;

	// 指定の階層を画面左寄りに置くためのスクロール量
	float CalcScroll(int focusFloor) const;

	// ノードの画面座標
	void GetNodePos(int floor, int index, int& x, int& y) const;
};