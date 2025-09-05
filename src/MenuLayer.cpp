#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "ZenGardenLayer.hpp"

using namespace geode::prelude;

class $modify(ZenGardenMenuLayer, MenuLayer)
{
	bool init()
	{
		if (!MenuLayer::init())
		{
			log::error("Failed to initialize MenuLayer!");
			return false;
		}

		CCSize marigoldSize{20.0f, 10.0f};

	auto marigoldSprite = CCSprite::create("Marigold.png"_spr);

	auto openZenGarden = CCMenuItemSpriteExtra::create(
		CircleButtonSprite::create(
			marigoldSprite,
			CircleBaseColor::Green,
			CircleBaseSize::MediumAlt),
		this,
		menu_selector(ZenGardenMenuLayer::onZenGarden));

	auto menu = this->getChildByID("right-side-menu");
	menu->addChild(openZenGarden);

	openZenGarden->setID("open-zen-garden");

	menu->updateLayout();

	GameStatsManager::sharedState()->setStat("29", Mod::get()->getSavedValue<int>("money", GameStatsManager::sharedState()->getStat("29")));

	return true;
	}

	void onZenGarden(CCObject *)
	{
		auto scene = CCScene::create();
		scene->addChild(ZenGardenLayer::create());

		CCDirector::get()->pushScene(CCTransitionFade::create(.5f, scene));
	}
};