#pragma once

// プレイヤーのステータス(Data/Player/Player.csv から読み込む)
// CSV に書かれていない項目は、ここのデフォルト値になる
struct PlayerParam
{
	// 設定ファイル(実行時のカレントディレクトリからの相対パス)
	static constexpr const char* FILE_PATH = "Data/CSV/Player.csv";

	int hp = 10;               // 最大HP
	float speedMove = 2.0f;    // 歩く速さ
	float speedRun = 10.0f;    // 走る速さ
	float timeRot = 1.0f;      // 振り向き完了までの時間(秒)
	float jumpPow = 80.0f;     // ジャンプ力

	// CSV を読み込んで返す(読めなければデフォルト値)
	static PlayerParam Load(void);
};
