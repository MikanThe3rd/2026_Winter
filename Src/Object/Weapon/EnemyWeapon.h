#pragma once
#include <string>
#include <DxLib.h>

// 武器1種類分の性能・見た目・判定の設定
struct WeaponData
{
	std::string modelPath;      // 武器モデル(.mv1)
	std::string handFrameName;  // 装着先のフレーム名(モデルによって異なる)
	VECTOR localPos;            // 手からのオフセット
	VECTOR localRot;            // 手からの回転(度)
	float  scale;               // 武器モデルの拡大率
	int    damage;              // プレイヤーに与えるダメージ
	float  range;               // 攻撃を開始できる距離
	int    cooldownFrames;      // 攻撃開始から次の攻撃までの待ち時間
	int    attackFrames;        // 攻撃モーション全体の長さ
	int    hitStartFrame;       // 判定が出始めるフレーム
	int    hitEndFrame;         // 判定が終わるフレーム
	float  hitLength;           // 武器ローカルのY方向に伸ばす判定の長さ
	float  hitRadius;           // 判定の半径(ワールド単位)
};

class EnemyWeapon
{
public:

	EnemyWeapon(const WeaponData& data, int ownerModelId);
	~EnemyWeapon(void);

	// アニメーション更新後に呼ぶこと(手のフレーム位置を使うため)
	void Update(void);
	void Draw(void);

	// 攻撃を開始できるか(dist は敵とプレイヤーの水平距離)
	bool CanAttack(float dist) const;
	void StartAttack(void);
	bool IsAttacking(void) const { return attackTimer_ > 0; }
	bool IsAttackEnd(void) const { return attackTimer_ <= 0; }

	// 判定が有効か(攻撃中・判定期間内・今回の一振りで未ヒット)
	bool IsHitActive(void) const;
	void MarkHit(void) { hasHit_ = true; }

	// 判定カプセル(ワールド座標)
	const VECTOR& GetPosTop(void) const { return posTop_; }
	const VECTOR& GetPosDown(void) const { return posDown_; }
	float GetRadius(void) const { return data_.hitRadius; }

	const WeaponData& GetData(void) const { return data_; }

	void EndAttack(void) { attackTimer_ = 0; }

private:

	WeaponData data_;
	int ownerModelId_;
	int modelId_;
	int frameIdx_;
	int cooldownTimer_;
	int attackTimer_;

	bool   hasHit_;
	VECTOR posTop_;
	VECTOR posDown_;
};