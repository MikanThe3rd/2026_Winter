#include "PostEffectBase.h"
#include "../Utility/AsoUtility.h"
#include "../Application.h"

PostEffectBase::PostEffectBase(void):
	vertexs_(),
	indexes_()
{
}

PostEffectBase::~PostEffectBase(void)
{
	if (shader_ != -1) DeleteShader(shader_);
	if (constBuf_ != -1) DeleteShaderConstantBuffer(constBuf_);
}

void PostEffectBase::Init(void) 
{
	Vector2 pos = Vector2(0, 0);
	Vector2 size = Vector2(
		Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y);

	int cnt = 0;
	float sX = static_cast<float>(pos.x);
	float sY = static_cast<float>(pos.y);
	float eX = static_cast<float>(pos.x + size.x);
	float eY = static_cast<float>(pos.y + size.y);

	// ÇSí∏ì_ÇÃèâä˙âª
	for (int i = 0; i < 4; i++)
	{
		vertexs_[i].rhw = 1.0f;
		vertexs_[i].dif = GetColorU8(255, 255, 255, 255);
		vertexs_[i].spc = GetColorU8(255, 255, 255, 255);
		vertexs_[i].su = 0.0f;
		vertexs_[i].sv = 0.0f;
	}

	// ç∂è„
	vertexs_[cnt].pos = VGet(sX, sY, 0.0f);
	vertexs_[cnt].u = 0.0f;
	vertexs_[cnt].v = 0.0f;
	cnt++;

	// âEè„
	vertexs_[cnt].pos = VGet(eX, sY, 0.0f);
	vertexs_[cnt].u = 1.0f;
	vertexs_[cnt].v = 0.0f;
	cnt++;

	//âEâ∫
	vertexs_[cnt].pos = VGet(eX, eY, 0.0f);
	vertexs_[cnt].u = 1.0f;
	vertexs_[cnt].v = 1.0f;
	cnt++;

	//ç∂â∫
	vertexs_[cnt].pos = VGet(sX, eY, 0.0f);
	vertexs_[cnt].u = 0.0f;
	vertexs_[cnt].v = 1.0f;
	cnt++;

	/*
	Å@Å`Å`Å`Å`Å`Å`
		0-----1
		|     |
		|     |
		3-----2
	Å@Å`Å`Å`Å`Å`Å`
		0-----1
		|  Å^
		|Å^
		3
	Å@Å`Å`Å`Å`Å`Å`
			  1
		   Å^ |
		 Å^   |
		3-----2
	Å@Å`Å`Å`Å`Å`Å`
	*/

	//í∏ì_ÉCÉìÉfÉbÉNÉX
	cnt = 0;
	indexes_[cnt++] = 0;
	indexes_[cnt++] = 1;
	indexes_[cnt++] = 3;

	indexes_[cnt++] = 1;
	indexes_[cnt++] = 2;
	indexes_[cnt++] = 3;
}

void PostEffectBase::Apply(int src, int dst)
{
	if (!isEnable_) {
		SetDrawScreen(dst);
		DrawGraph(0, 0, src, false);
		return;
	}

	SetDrawScreen(dst);
	ClearDrawScreen();
	MV1SetUseOrigShader(true);
	SetUsePixelShader(shader_);
	SetUseTextureToShader(0, src);

	SetConstantBuffer();

	UpdateShaderConstantBuffer(constBuf_);
	SetShaderConstantBuffer(constBuf_, DX_SHADERTYPE_PIXEL, CONSTANT_BUF_SLOT_BEGIN_PS);

	DrawPolygons();

	SetUseTextureToShader(0, -1);
	SetUsePixelShader(-1);
	MV1SetUseOrigShader(false);
}

void PostEffectBase::DrawPolygons()
{
	DrawPolygonIndexed2DToShader(vertexs_, NUM_VERTEX, indexes_, NUM_POLYGON);
}
