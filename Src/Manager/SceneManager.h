#pragma once
#include <memory>
#include <chrono>
#include <stack>

// 推奨しませんが、どうしても使いたい方は
#define mainCamera SceneManager::GetInstance().GetCamera()

class SceneBase;
class Fader;
class Camera;

class SceneManager
{

public:

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		MAP,
		GAME,
		EVENT,
		GAME_OVER,
		GAME_CLEAR,
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	void Init(void);
	void Init3D(void);
	void Update(void);
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	// カメラの取得
	Camera& GetCamera(void);

	int GetMainScreen(void);

	bool isEnd_;

	bool IsEnd(void) { return isEnd_; }

	void PushScene(SceneBase* scene);
	void PopScene(void);

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;


	// 各種シーン
	std::unique_ptr<SceneBase> scene_;

	// フェード
	std::unique_ptr<Fader> fader_;

	// カメラ
	std::unique_ptr<Camera> camera_;

	std::stack<SceneBase*> stackScene_;

	// ★変更: PopSceneの予約フラグ(Update中に自分自身をdeleteしないため)
	bool popRequested_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	//メインスクリーン
	int mainScreen_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);
	// コピーコンストラクタも同様
	SceneManager(const SceneManager& manager) = default;
	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);

	void ClearStack(void);

};