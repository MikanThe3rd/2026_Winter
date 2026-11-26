#pragma once
#include <vector>
#include <DxLib.h>
#include "EnemyAttackBase.h"

// ñÇñ@(âìãóó£íe)1éÌóﬁï™ÇÃê›íË
struct MagicData
{
	float        range;           // çUåÇÇäJénÇ≈Ç´ÇÈãóó£
	int          damage;          // ÉvÉåÉCÉÑÅ[Ç…ó^Ç¶ÇÈÉ_ÉÅÅ[ÉW
	int          cooldownFrames;  // çUåÇäJénÇ©ÇÁéüÇÃçUåÇÇ‹Ç≈ÇÃë“Çøéûä‘
	int          castFrames;      // ârè•ÉÇÅ[ÉVÉáÉìëSëÃÇÃí∑Ç≥
	int          fireFrame;       // íeÇî≠éÀÇ∑ÇÈÉtÉåÅ[ÉÄ(StartAttack Ç©ÇÁÇÃåoâﬂ)
	float        bulletSpeed;     // íeë¨(1ÉtÉåÅ[ÉÄÇ†ÇΩÇË)
	float        bulletRadius;    // íeÇÃîºåa(å©ÇΩñ⁄ÅEîªíËã§í )
	int          bulletLife;      // íeÇÃéıñΩ(ÉtÉåÅ[ÉÄ)
	float        muzzleHeight;    // î≠éÀà íuÇÃçÇÇ≥(ìGÇÃë´å≥Ç©ÇÁ)
	float        aimHeight;       // ë_Ç§çÇÇ≥(ÉvÉåÉCÉÑÅ[ÇÃë´å≥Ç©ÇÁ)
	unsigned int color;           // íeÇÃêF
};

class EnemyMagic : public EnemyAttackBase
{
public:

	explicit EnemyMagic(const MagicData& data);
	~EnemyMagic(void) override = default;

	void Update(const Context& ctx) override;
	void Draw(void) override;

	bool CanAttack(float dist) const override;
	void StartAttack(void) override;

	// ârè•ÇíÜífÇ∑ÇÈÅBî≠éÀçœÇ›ÇÃíeÇÕîÚÇ—ë±ÇØÇÈ
	void EndAttack(void) override { attackTimer_ = 0; }
	bool IsAttacking(void) const override { return attackTimer_ > 0; }

	int CheckHitPlayer(const Capsule& player) override;

private:

	struct Bullet
	{
		VECTOR pos;
		VECTOR vel;
		int    life;
		bool   hit;
	};

	void Fire(const Context& ctx);

	MagicData data_;
	int  cooldownTimer_;
	int  attackTimer_;
	bool fired_;                  // ç°âÒÇÃârè•Ç≈î≠éÀçœÇ›Ç©
	std::vector<Bullet> bullets_;
};