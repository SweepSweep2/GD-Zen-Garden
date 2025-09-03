#include "ZenGardenShopLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>

ZenGardenShopLayer *ZenGardenShopLayer::create() {
    auto zenGardenShopLayer = new ZenGardenShopLayer();

    if (zenGardenShopLayer && zenGardenShopLayer->init())
    {
        zenGardenShopLayer->autorelease();
        return zenGardenShopLayer;
    }

    CC_SAFE_DELETE(zenGardenShopLayer);
    return nullptr;
}

void ZenGardenShopLayer::onBack(CCObject *sender) {
    CCDirector::sharedDirector()->popScene();
}

void ZenGardenShopLayer::onBuyStars(CCObject *sender) {
    geode::createQuickPopup(
        "Buy Stars",
        "Are you sure you want to buy 5 <cy>stars</c>?",
        "Yes", "No",
        [this](auto, bool btn2)
        {
            if (!btn2)
            {
                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cy>stars</c>!",
                    "OK"
                )->show();
            }
        });
}

void ZenGardenShopLayer::onBuyMoons(CCObject *sender) {
    geode::createQuickPopup(
        "Buy Moons",
        "Are you sure you want to buy 5 <cb>moons</c>?",
        "Yes", "No",
        [this](auto, bool btn2)
        {
            if (!btn2)
            {
                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cb>moons</c>!",
                    "OK"
                )->show();
            }
        });
}

void ZenGardenShopLayer::onBuyDiamonds(CCObject *sender) {
    geode::createQuickPopup(
        "Buy Diamonds",
        "Are you sure you want to buy 5 <cb>diamonds</c>?",
        "Yes", "No",
        [this](auto, bool btn2)
        {
            if (!btn2)
            {
                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cb>diamonds</c>!",
                    "OK"
                )->show();
            }
        });
}

bool ZenGardenShopLayer::init() {
    if (!CCLayer::init())
    {
        return false;
    }

    // Update the icons label to refresh the restock timer every second
    this->schedule(schedule_selector(ZenGardenShopLayer::updateIconsLabel), 1.0f);

    auto windowSize = CCDirector::sharedDirector()->getWinSize();

    this->setID("zen-garden-shop");

    // h
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GJ_ShopSheet01-uhd.plist");
    auto background = CCSprite::createWithSpriteFrameName("shopBG_001.png");

    // Custom background (change if there is a better way to do this)
    background->setScaleX(windowSize.width / 480);
    background->setScaleY(windowSize.height / 320);
    background->setPosition(ccp(windowSize.width / 2, windowSize.height / 2));
    background->setID("background");

    this->addChild(background, -5);

    auto exitMenu = CCMenu::create();
    exitMenu->setID("exit-menu");

    this->addChild(exitMenu);

    auto close = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(ZenGardenShopLayer::onBack));

    close->setPosition((windowSize.width / 2) - 32, (windowSize.height / 2) - 32);
    close->setID("exit-zen-garden-shop");

    exitMenu->addChild(close);

    auto shopLogoSprite = CCSprite::create("Shop.png"_spr);

    shopLogoSprite->setPosition(ccp(windowSize.width / 2, windowSize.height - 35));
    shopLogoSprite->setID("shop-logo");

    this->addChild(shopLogoSprite);

    // Get the local time
    auto now = std::chrono::system_clock::now();

    // Convert time to time_t and then to tm struct
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&tt);

    // Set the time to tomorrow at midnight
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    local_tm.tm_mday += 1;

    // Convert back to a time_point
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(&local_tm));

    // Calculate the difference
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(midnight - now);

    // Convert difference to hours, minutes, and seconds
    int hours = diff.count() / 3600;
    int minutes = (diff.count() % 3600) / 60;
    int seconds = diff.count() % 60;

    // Create the label
    ZenGardenShopLayer::m_iconsLabel = CCLabelBMFont::create(
        (std::string("Icons (restocks in ") 
        + (hours < 10 ? "0" : "") + std::to_string(hours) + ":"
        + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":"
        + (seconds < 10 ? "0" : "") + std::to_string(seconds) + ")").c_str(),
        "bigFont.fnt"
    );

    // t i m e

    ZenGardenShopLayer::m_iconsLabel->setPosition(windowSize.width / 2, windowSize.height - 105);
    ZenGardenShopLayer::m_iconsLabel->setID("icons-label");
    this->addChild(ZenGardenShopLayer::m_iconsLabel);

    auto iconSlot1 = CCSprite::create("GJ_button_05.png");
    iconSlot1->setPosition(ccp(windowSize.width / 2 - 82, windowSize.height - 155));
    iconSlot1->setScale(1.25f);
    iconSlot1->setID("icon-slot-1");

    this->addChild(iconSlot1);

    auto iconSlot2 = CCSprite::create("GJ_button_05.png");
    iconSlot2->setPosition(ccp(windowSize.width / 2 + 82, windowSize.height - 155));
    iconSlot2->setScale(1.25f);
    iconSlot2->setID("icon-slot-2");

    this->addChild(iconSlot2);

    auto statsLabel = CCLabelBMFont::create("Stars/Moons/Diamonds", "bigFont.fnt");
    statsLabel->setPosition(windowSize.width / 2, windowSize.height - 225);
    statsLabel->setID("stats-label");

    this->addChild(statsLabel);

    auto buyStatsMenu = CCMenu::create();
    buyStatsMenu->setID("buy-stats-menu");

    this->addChild(buyStatsMenu);

    auto buyStars = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_starsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenShopLayer::onBuyStars));

    buyStars->setID("buy-stars");
    buyStars->setPosition(windowSize.width / 2 - 350, windowSize.height - 430);
    buyStars->m_baseScale = 1.25f;
    buyStars->setScale(1.25f);

    buyStatsMenu->addChild(buyStars);

    this->addChild(buyStatsMenu);

    auto buyMoons = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_moonsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenShopLayer::onBuyMoons));

    buyMoons->setID("buy-moons");
    buyMoons->setPosition(windowSize.width / 2 - 284.5, windowSize.height - 430);
    buyMoons->m_baseScale = 1.25f;
    buyMoons->setScale(1.25f);

    buyStatsMenu->addChild(buyMoons);

    auto buyDiamonds = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_diamondsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenShopLayer::onBuyDiamonds));

    buyDiamonds->setID("buy-diamonds");
    buyDiamonds->setPosition(windowSize.width / 2 - 219, windowSize.height - 430);
    buyDiamonds->m_baseScale = 1.25f;
    buyDiamonds->setScale(1.25f);

    buyStatsMenu->addChild(buyDiamonds);

    return true;
}

void ZenGardenShopLayer::updateIconsLabel(float dt) {
    // Get the local time
    auto now = std::chrono::system_clock::now();

    // Convert time to time_t and then to tm struct
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&tt);

    // Set the time to tomorrow at midnight
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    local_tm.tm_mday += 1;

    // Convert back to a time_point
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(&local_tm));

    // Calculate the difference
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(midnight - now);

    // Convert difference to hours, minutes, and seconds
    int hours = diff.count() / 3600;
    int minutes = (diff.count() % 3600) / 60;
    int seconds = diff.count() % 60;

    // Update the label text
    ZenGardenShopLayer::m_iconsLabel->setString((
        std::string("Icons (restocks in " )
        + (hours < 10 ? "0" : "") + std::to_string(hours) + ":"
        + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":"
        + (seconds < 10 ? "0" : "") + std::to_string(seconds) + ")").c_str()
    );

    // t i m e
}