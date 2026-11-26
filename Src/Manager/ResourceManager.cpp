#include <DxLib.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{

	// 推奨しませんが、どうしても使いたい方は
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;

	std::unique_ptr<Resource> res;

	// タイトル画像
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Title.png");
	resourcesMap_.emplace(SRC::TITLE, std::move(res));

	// PushSpace
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "PushSpace.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, std::move(res));

	//通常戦闘のアイコン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Normal.png");
	resourcesMap_.emplace(SRC::BATTLE_ICON_NORMAL, std::move(res));

	//強敵戦闘のアイコン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Elite.png");
	resourcesMap_.emplace(SRC::BATTLE_ICON_ELITE, std::move(res));

	//ボス戦闘のアイコン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Boss.png");
	resourcesMap_.emplace(SRC::BATTLE_ICON_BOSS, std::move(res));

	//イベントのアイコン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Event.png");
	resourcesMap_.emplace(SRC::EVENT_ICON, std::move(res));

	//休憩のアイコン
	res = std::make_unique<RES>(RES_T::IMG, PATH_IMG + "Rest.png");
	resourcesMap_.emplace(SRC::REST_ICON, std::move(res));

	// プレイヤー
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Player/vampire.mv1");
	resourcesMap_.emplace(SRC::PLAYER, std::move(res));

	// スカイドーム
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "SkyDome/SkyDome.mv1");
	resourcesMap_.emplace(SRC::SKY_DOME, std::move(res));

	// 森
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Forest.mv1");
	resourcesMap_.emplace(SRC::FOREST, std::move(res));

	// 街
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/CityDraw.mv1");
	resourcesMap_.emplace(SRC::CITY, std::move(res));

	// 街の当たり判定用
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/CityCollider.mv1");
	resourcesMap_.emplace(SRC::CITY_COLLISION, std::move(res));

	// 礼拝堂
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Stage/Chapel.mv1");
	resourcesMap_.emplace(SRC::CHAPEL, std::move(res));

	// 剣
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Weapon/Sword.mv1");
	resourcesMap_.emplace(SRC::SWORD, std::move(res));

	// 爪
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Weapon/Claw.mv1");
	resourcesMap_.emplace(SRC::CLAW, std::move(res));

	// ナイト
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Knight/Knight.mv1");
	resourcesMap_.emplace(SRC::KNIGHT, std::move(res));

	// シスター
	res = std::make_unique<RES>(RES_T::MODEL, PATH_MDL + "Sister/Sister.mv1");
	resourcesMap_.emplace(SRC::SISTER, std::move(res));
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{

	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return lPair->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;

}
