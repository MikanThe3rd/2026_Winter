#pragma once
#include <string>
#include <DxLib.h>

// 外部ファイル(CSV)から読み込むエネミー1体分の設定
// 省略された項目はここのデフォルト値が使われる
struct EnemySpawnData
{
	std::string type = "Knight";                  // エネミー種別
	VECTOR pos = { 0.0f, 0.0f, 0.0f };            // スポーン位置
	float rotY = 0.0f;                            // 初期向き(度)

	float speedMove = 1.5f;                       // 移動速度
	float detectRange = 600.0f;                   // 追従開始距離
	float loseRange = 900.0f;                     // 追従終了距離
	float stopRange = 80.0f;                      // 近付く限界距離
	int hp = 3;                                   // ヒットポイント
};