#pragma once
#include<DxLib.h>
#include<string>

class PostEffectBase
{
public:
	//コンストラクタ
	PostEffectBase(void);

	//デストラクタ
	~PostEffectBase(void);

	virtual void Init(void);
	virtual void Update(void) {};

	void UpdateIfEnabled(void) {
		if (isEnable_) {
			Update();
		}
	}

	virtual void Apply(int src, int dst);

	void SetEnabled(bool enabled) { isEnable_ = enabled; }
	bool IsEnabled(void) const { return isEnable_; }

	// ピクセルシェーダ用オリジナル定数バッファの使用開始スロット
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_PS = 4;

	// 頂点数
	static constexpr int NUM_VERTEX = 4;

	// 頂点インデックス数
	static constexpr int NUM_VERTEX_IDX = 6;

	// ポリゴン数
	static constexpr int NUM_POLYGON = 2;

protected:
	// 定数バッファのセット
	virtual void SetConstantBuffer() = 0;

	virtual void DrawPolygons();

	int shader_ = -1;
	int constBuf_ = -1;

	VERTEX2DSHADER vertexs_[NUM_VERTEX] = {};
	WORD           indexes_[NUM_VERTEX_IDX] = {};

private:

	bool isEnable_ = false;
};