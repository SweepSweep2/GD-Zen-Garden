#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/LazySprite.hpp>
#include "ZenGardenLayer.hpp"

using namespace geode::prelude;

class $modify(ZenGardenMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) {
			log::error("Failed to initialize MenuLayer!");
			return false;
		}
		
		CCSize marigoldSize{20.0f, 10.0f};

		auto marigoldSprite = LazySprite::create(marigoldSize);

		marigoldSprite->setLoadCallback([marigoldSprite](Result<> res) {
			if (res) {
				log::info("Sprite loaded successfully!");
			} else {
				log::error("Sprite failed to load, setting fallback: {}", res.unwrapErr());
			}
		});

		marigoldSprite->loadFromFile(Mod::get()->getResourcesDir() / "Marigold.png");

		auto openZenGarden = CCMenuItemSpriteExtra::create(
			geode::CircleButtonSprite::create(
				marigoldSprite,
				geode::CircleBaseColor::Green,
				geode::CircleBaseSize::Medium
			),
			this,
			menu_selector(ZenGardenMenuLayer::onZenGarden)
		);

		auto menu = this->getChildByID("right-side-menu");
		menu->addChild(openZenGarden);

		openZenGarden->setID("open-zen-garden");

		menu->updateLayout();

		return true;
	}

	void onZenGarden(CCObject*) {
		auto scene = cocos2d::CCScene::create();
		scene->addChild(ZenGardenLayer::create());

		CCDirector::sharedDirector()->pushScene(scene);
	}
};