#include "ZenGardenLayer.hpp"
#include <Geode/Geode.hpp>
#include <chrono>

struct SaveData {
    int stars;
    int moons;
    int diamonds;
    int money;
    long long timeSinceLastLoad;
};

SaveData saveData;

template<>
struct matjson::Serialize<SaveData> {
    static SaveData from_json(matjson::Value const& value) {
        SaveData data;
        
        data.stars = value["stars"].as<int>().unwrapOr(0);
        data.moons = value["moons"].as<int>().unwrapOr(0);
        data.diamonds = value["diamonds"].as<int>().unwrapOr(0);
        data.money = value["money"].as<int>().unwrapOr(0);
        data.timeSinceLastLoad = value["timeSinceLastLoad"].as<long long>().unwrapOr(0);
        
        return data;
    }

    static matjson::Value to_json(SaveData const& value) {
        auto obj = matjson::makeObject(
            {
                {"stars", value.stars},
                {"moons", value.moons},
                {"diamonds", value.diamonds},
                {"money", value.money},
                {"timeSinceLastLoad", value.timeSinceLastLoad}
            }
        );

        return obj;
    }
};

void ZenGardenLayer::onClose(CCObject* sender) {
    Mod::get()->setSavedValue<int>("stars", ZenGardenLayer::m_starCount);

    CCDirector::sharedDirector()->popScene();
}

void ZenGardenLayer::onFeedOrbs(CCObject* sender) {
    if (ZenGardenLayer::m_selectedItem != 1) {
        ZenGardenLayer::m_selectedItem = 1;
        ZenGardenLayer::m_orbsHoverSprite->setVisible(true);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedStars(CCObject* sender) {
    if (ZenGardenLayer::m_selectedItem != 2) {
        ZenGardenLayer::m_selectedItem = 2;
        ZenGardenLayer::m_starsHoverSprite->setVisible(true);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedMoons(CCObject* sender) {
    if (ZenGardenLayer::m_selectedItem != 3) {
        ZenGardenLayer::m_selectedItem = 3;
        ZenGardenLayer::m_moonsHoverSprite->setVisible(true);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedDiamonds(CCObject* sender) {
    if (ZenGardenLayer::m_selectedItem != 4) {
        ZenGardenLayer::m_selectedItem = 4;
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(true);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
}

bool ZenGardenLayer::init() {
    if (!CCLayer::init()) return false;

    this->scheduleUpdate();

    this->setID("zen-garden");

    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190};

    ZenGardenLayer::m_starCount = Mod::get()->getSavedValue<int>("stars");

    auto background = CCSprite::create("GJ_gradientBG.png");
    background->setScaleX(windowSize.width / background->getContentSize().width);
    background->setScaleY(windowSize.height / background->getContentSize().height);
    background->setPosition(ccp(284.5f, 160.0f));
    background->setID("background");
    background->setColor({0, 102, 255});

    this->addChild(background);

    auto exitMenu = CCMenu::create();
    exitMenu->setID("exit-menu");

    this->addChild(exitMenu);

    auto close = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(ZenGardenLayer::onClose)
    );

    close->setPosition((windowSize.width / 2) - 32, (windowSize.height / 2) - 32);
    close->setID("exit-zen-garden");

    exitMenu->addChild(close);

    auto topBarMenu = CCMenu::create();
    topBarMenu->setID("top-bar-menu");

    this->addChild(topBarMenu);

    auto topBarBG = CCScale9Sprite::create("square02b_001.png");
    topBarBG->setContentSize({163, 40});
    topBarBG->setPosition((windowSize.width / -2) + 84.5, (windowSize.height / 2) - 25);
    topBarBG->setID("top-bar-bg");
    topBarBG->setColor({0, 36, 91});

    topBarMenu->addChild(topBarBG);

    auto feedOrbsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "currencyOrbIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}
        ),
        this,
        menu_selector(ZenGardenLayer::onFeedOrbs)
    );

    feedOrbsButton->setID("feed-orbs");
    feedOrbsButton->setPosition((windowSize.width / -2) + 25.5, (windowSize.height / 2) - 25);

    topBarMenu->addChild(feedOrbsButton);

    ZenGardenLayer::m_orbsHoverSprite = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
    ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    ZenGardenLayer::m_orbsHoverSprite->setID("orbs-hover");

    this->addChild(ZenGardenLayer::m_orbsHoverSprite);

    auto orbCount = CCLabelBMFont::create("8", "bigFont.fnt");
    orbCount->setID("orb-count");
    orbCount->setPosition((windowSize.width / -2) + 35.5, (windowSize.height / 2) - 34);
    orbCount->setRotation(90.0f);
    orbCount->setScale(0.5f);

    topBarMenu->addChild(orbCount);

    auto feedStarsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_starsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}
        ),
        this,
        menu_selector(ZenGardenLayer::onFeedStars)
    );

    feedStarsButton->setID("feed-stars");
    feedStarsButton->setPosition((windowSize.width / -2) + 65, (windowSize.height / 2) - 25);

    topBarMenu->addChild(feedStarsButton);

    ZenGardenLayer::m_starsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    ZenGardenLayer::m_starsHoverSprite->setVisible(false);
    ZenGardenLayer::m_starsHoverSprite->setID("stars-hover");
    
    this->addChild(ZenGardenLayer::m_starsHoverSprite);

    ZenGardenLayer::m_starsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_starCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_starsLabel->setID("star-count");
    ZenGardenLayer::m_starsLabel->setPosition((windowSize.width / -2) + 75, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_starsLabel->setScale(0.5f);

    topBarMenu->addChild(ZenGardenLayer::m_starsLabel);

    auto feedMoonsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_moonsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}
        ),
        this,
        menu_selector(ZenGardenLayer::onFeedMoons)
    );

    feedMoonsButton->setID("feed-moons");
    feedMoonsButton->setPosition((windowSize.width / -2) + 104.5, (windowSize.height / 2) - 25);

    topBarMenu->addChild(feedMoonsButton);

    ZenGardenLayer::m_moonsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
    ZenGardenLayer::m_moonsHoverSprite->setID("moons-hover");

    this->addChild(ZenGardenLayer::m_moonsHoverSprite);

    ZenGardenLayer::m_moonsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_moonCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_moonsLabel->setID("moon-count");
    ZenGardenLayer::m_moonsLabel->setPosition((windowSize.width / -2) + 114.5, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_moonsLabel->setScale(0.5f);

    topBarMenu->addChild(ZenGardenLayer::m_moonsLabel);

    auto feedDiamondsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_diamondsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}
        ),
        this,
        menu_selector(ZenGardenLayer::onFeedDiamonds)
    );

    feedDiamondsButton->setID("feed-diamonds");
    feedDiamondsButton->setPosition((windowSize.width / -2) + 144, (windowSize.height / 2) - 25);

    topBarMenu->addChild(feedDiamondsButton);

    ZenGardenLayer::m_diamondsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
    ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    ZenGardenLayer::m_diamondsHoverSprite->setID("diamonds-hover");

    this->addChild(ZenGardenLayer::m_diamondsHoverSprite);

    ZenGardenLayer::m_diamondsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_diamondCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_diamondsLabel->setID("diamond-count");
    ZenGardenLayer::m_diamondsLabel->setPosition((windowSize.width / -2) + 154, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_diamondsLabel->setScale(0.5f);

    topBarMenu->addChild(ZenGardenLayer::m_diamondsLabel);

    ZenGardenLayer::m_moneyLabel = CCLabelBMFont::create("$1000", "bigFont.fnt");
    ZenGardenLayer::m_moneyLabel->setID("money-count");
    ZenGardenLayer::m_moneyLabel->setPosition(windowSize.width / 2, windowSize.height - 300);

    this->addChild(ZenGardenLayer::m_moneyLabel);

    auto iconBackgrounds = CCSpriteBatchNode::create("GJ_button_05.png");
    iconBackgrounds->setID("icon-backgrounds");
    
    int x;
    int y;
    CCSprite* iconBackgroundSprite;
    int test;

    for (int i = 0; i < 32; i++) {
        x = xPositions[i % 8];

        if (i < 8) {
            y = 75;
        } else if (i < 16) {
            y = 25;
        } else if (i < 24) {
            y = -25;
        } else if (i < 32) {
            y = -75;
        }

        x = (windowSize.width / 2) + x;
        y = (windowSize.height / 2) + y;

        iconBackgroundSprite = CCSprite::create("GJ_button_05.png");
        iconBackgroundSprite->setPosition(ccp(x, y));

        iconBackgrounds->addChild(iconBackgroundSprite);
    }

    this->addChild(iconBackgrounds);

    return true;
}

void ZenGardenLayer::update(float dt) {
    if (ZenGardenLayer::m_selectedItem != 0) {
        switch (m_selectedItem) {
            case 1:
                ZenGardenLayer::m_orbsHoverSprite->setPosition(getMousePos());
            case 2:
                ZenGardenLayer::m_starsHoverSprite->setPosition(getMousePos());
            case 3:
                ZenGardenLayer::m_moonsHoverSprite->setPosition(getMousePos());
            case 4:
                ZenGardenLayer::m_diamondsHoverSprite->setPosition(getMousePos());
        }
    }

    ZenGardenLayer::m_starsLabel->setString(std::to_string(ZenGardenLayer::m_starCount).c_str(), true);
    ZenGardenLayer::m_moonsLabel->setString(std::to_string(ZenGardenLayer::m_moonCount).c_str(), true);
    ZenGardenLayer::m_diamondsLabel->setString(std::to_string(ZenGardenLayer::m_diamondCount).c_str(), true);
    ZenGardenLayer::m_moneyLabel->setString(("$" + std::to_string(ZenGardenLayer::m_money)).c_str(), true);
}

ZenGardenLayer *ZenGardenLayer::create() {
    auto zenGardenLayer = new ZenGardenLayer();

    if (zenGardenLayer && zenGardenLayer->init()) {
        zenGardenLayer->autorelease();
        return zenGardenLayer;
    }

    delete zenGardenLayer;
    return nullptr;
};