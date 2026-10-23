#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <DxLib.h>
#include "../Common/Transform.h"
#include "EnemySpawnData.h"

class ResourceManager;
class Collider;
class Capsule;
class AnimationController;
class EnemySword;

class EnemyBase
{
public:

	enum class STATE
	{
		SEARCH,
		TRACKING,
		ENGAGE,     
		STUN,      
		ATTACK,
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		ATTACK,         
		BACK,    
		STUN,          
	};

	// 交戦中の行動
	enum class ENGAGE_ACTION
	{
		BACK,
		APPROACH,
		ATTACK,
	};

	static constexpr int HP_BAR_WIDTH = 60;    
	static constexpr int HP_BAR_HEIGHT = 6;		

	// 追加: 交戦まわりの調整値(フレーム単位)
	static constexpr double TURN_RATE = 0.1;           
	static constexpr float  ENGAGE_EXIT_MARGIN = 60.0f; 
	static constexpr int    ENGAGE_TIME_MIN = 30;       
	static constexpr int    ENGAGE_TIME_RAND = 40;     
	// 行動の確率(累積 %)
	static constexpr int    RATE_ATTACK = 35;
	static constexpr int    RATE_BACK = 65;             

	// スタン
	static constexpr int    STUN_HP_THRESHOLD = 3;      
	static constexpr int    STUN_TIME = 600;          

	// コンストラクタ
	EnemyBase(void);

	// デストラクタ
	virtual ~EnemyBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void);
	virtual void Draw(void);

	virtual void InitAnimation(void) = 0;

	void Damage(int damage);

	bool IsAlive(void) const { return hp_ > 0; }
	int GetHp(void) const { return hp_; }
	int GetHpMax(void) const { return hpMax_; }

	// スタン中か
	bool IsStunned(void) const { return state_ == STATE::STUN; }

	const Transform& GetTransform(void) const { return transform_; }

	// 衝突用カプセルの取得
	const Capsule& GetCapsule(void) const { return *capsule_; }

	// 衝突判定に用いられるコライダ制御(Stage から呼ばれる)
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

	// 追従対象の設定(所有しない)
	void SetTarget(const Transform* target) { target_ = target; }

	void SetSpawnData(const EnemySpawnData& data) { spawnData_ = data, hp_ = hpMax_ = data.hp; }

	// 武器の装備(Init() の後、モデル読み込み済みの状態で呼ぶ)
	void EquipWeapon(const std::string& weaponName);

	// 武器の有無・取得(武器なしなら nullptr)
	bool HasWeapon(void) const { return weapon_ != nullptr; }
	EnemySword* GetWeapon(void) { return weapon_.get(); }

	//重なった場合の押しだし
	void AddPos(const VECTOR& v);

protected:

	EnemySpawnData spawnData_;


	// シングルトン参照
	ResourceManager& resMng_;

	Transform transform_;

	// 追従対象
	const Transform* target_;

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;
	std::unique_ptr<Capsule> capsule_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;

	// 武器
	std::unique_ptr<EnemySword> weapon_;

	// 状態管理
	STATE state_;
	
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// 移動量
	VECTOR movePow_;

	// 重力・ジャンプによる移動量
	VECTOR jumpPow_;

	// 移動後の座標
	VECTOR movedPos_;

	// 衝突チェック
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// ヒットポイント
	int hp_;
	int hpMax_;

	// HPバーの表示位置オフセット
	float hpBarOffsetY_;

	// 交戦中の現在の行動と残りフレーム
	ENGAGE_ACTION engageAction_;
	int engageTimer_;

	// スタンの残りフレームと、発動済みフラグ(1体につき1回だけ)
	int stunTimer_;
	bool stunTriggered_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateSearch(void);
	void ChangeStateTracking(void);
	void ChangeStateEngage(void);       
	void ChangeStateStun(void);         
	void ChangeStateAttack(void);

	virtual void UpdateSearch(void) = 0;
	virtual void UpdateTracking(void) = 0;
	virtual void UpdateEngage(void);    
	virtual void UpdateStun(void);      
	virtual void UpdateAttack(void);

	// 交戦中の行動を抽選する(攻撃に遷移したら state_ が ATTACK になる)
	void PickEngageAction(float dist);

	// 水平方向 dir へ徐々に振り向く
	void TurnToDir(const VECTOR& dir);

	// 武器があり攻撃可能なら ATTACK に遷移する
	bool TryStartAttack(float dist);

	// HPバーの描画
	virtual void DrawHpBar(void);

	// 重力量の計算
	void CalcGravityPow(void);

	// 衝突判定
	void Collision(void);
	void CollisionCapsule(void);
	void CollisionGravity(void);
};