#include "Ground.h"

Ground::Ground(const Stage::NAME& name, const Transform& transform)
{
	name_ = name;
	transform_ = transform;

	gravityPow_ = 0.0f;
	gravityRadius_ = 0.0f;
	deadLength_ = 0.0f;
}

Ground::~Ground(void)
{
}

void Ground::Init(void)
{
	gravityPow_ = DEFAULT_GRAVITY_POW;
	gravityRadius_ = DEFAULT_GRAVITY_RADIUS;
	deadLength_ = DEFAULT_DEAD_LENGTH;
}

void Ground::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}
