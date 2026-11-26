#include <algorithm>
#include "../../Common/Capsule.h"
#include "EnemyMagic.h"

EnemyMagic::EnemyMagic(const MagicData& data)
	:
	data_(data),
	cooldownTimer_(0),
	attackTimer_(0),
	fired_(false)
{
}

void EnemyMagic::Update(const Context& ctx)
{
	if (cooldownTimer_ > 0) { cooldownTimer_--; }
	if (attackTimer_ > 0) { attackTimer_--; }

	// ‰r¥’†A”­ËƒtƒŒ[ƒ€‚É’B‚µ‚½‚ç’e‚ğo‚·
	if (attackTimer_ > 0 && !fired_)
	{
		int elapsed = data_.castFrames - attackTimer_;
		if (elapsed >= data_.fireFrame)
		{
			Fire(ctx);
			fired_ = true;
		}
	}

	// ’e‚ÌˆÚ“®‚Æõ–½ŠÇ—(‚¿å‚ªUŒ‚’†‚Å‚È‚­‚Ä‚à“®‚©‚·)
	for (auto& b : bullets_)
	{
		b.pos = VAdd(b.pos, b.vel);
		b.life--;
	}
	bullets_.erase(
		std::remove_if(bullets_.begin(), bullets_.end(),
			[](const Bullet& b) { return b.life <= 0 || b.hit; }),
		bullets_.end());
}

void EnemyMagic::Draw(void)
{
	for (const auto& b : bullets_)
	{
		DrawSphere3D(b.pos, data_.bulletRadius, 16, data_.color, data_.color, TRUE);
	}
}

bool EnemyMagic::CanAttack(float dist) const
{
	return cooldownTimer_ <= 0 && attackTimer_ <= 0 && dist <= data_.range;
}

void EnemyMagic::StartAttack(void)
{
	attackTimer_ = data_.castFrames;
	cooldownTimer_ = data_.cooldownFrames;
	fired_ = false;
}

void EnemyMagic::Fire(const Context& ctx)
{
	VECTOR from = ctx.ownerPos;
	from.y += data_.muzzleHeight;

	VECTOR to = ctx.targetPos;
	to.y += data_.aimHeight;

	// ”­Ë‚µ‚½uŠÔ‚ÌˆÊ’u‚Ö’¼i‚·‚é(’Ç”ö‚µ‚È‚¢)
	VECTOR dir = VSub(to, from);
	if (VSquareSize(dir) < 0.0001f)
	{
		dir = VGet(0.0f, 0.0f, 1.0f);
	}
	dir = VNorm(dir);

	Bullet b;
	b.pos = from;
	b.vel = VScale(dir, data_.bulletSpeed);
	b.life = data_.bulletLife;
	b.hit = false;
	bullets_.push_back(b);
}

int EnemyMagic::CheckHitPlayer(const Capsule& player)
{
	int damage = 0;

	for (auto& b : bullets_)
	{
		if (b.hit)
		{
			continue;
		}

		int hit = HitCheck_Sphere_Capsule(
			b.pos, data_.bulletRadius,
			player.GetPosTop(), player.GetPosDown(), player.GetRadius());

		if (hit)
		{
			b.hit = true;   // “–‚½‚Á‚½’e‚ÍŸ‚Ì Update ‚ÅÁ‚¦‚é
			damage += data_.damage;
		}
	}
	return damage;
}