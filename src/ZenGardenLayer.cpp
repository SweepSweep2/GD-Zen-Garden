#include "ZenGardenLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>

struct SaveData
{
    int stars;
    int moons;
    int diamonds;
    int money;
    long long timeSinceLastLoad;
};

SaveData saveData;

template <>
struct matjson::Serialize<SaveData>
{
    static SaveData from_json(matjson::Value const &value)
    {
        SaveData data;

        data.stars = value["stars"].as<int>().unwrapOr(0);
        data.moons = value["moons"].as<int>().unwrapOr(0);
        data.diamonds = value["diamonds"].as<int>().unwrapOr(0);
        data.money = value["money"].as<int>().unwrapOr(0);
        data.timeSinceLastLoad = value["timeSinceLastLoad"].as<long long>().unwrapOr(0);

        return data;
    }

    static matjson::Value to_json(SaveData const &value)
    {
        auto obj = matjson::makeObject(
            {{"stars", value.stars},
             {"moons", value.moons},
             {"diamonds", value.diamonds},
             {"money", value.money},
             {"timeSinceLastLoad", value.timeSinceLastLoad}});

        return obj;
    }
};

ZenGardenLayer *ZenGardenLayer::create()
{
    auto zenGardenLayer = new ZenGardenLayer();

    if (zenGardenLayer && zenGardenLayer->init())
    {
        zenGardenLayer->autorelease();
        return zenGardenLayer;
    }

    CC_SAFE_DELETE(zenGardenLayer);
    return nullptr;
}

void ZenGardenLayer::onBack(CCObject *sender)
{
    Mod::get()->setSavedValue<int>("stars", ZenGardenLayer::m_starCount);
    Mod::get()->setSavedValue<int>("moons", ZenGardenLayer::m_moonCount);
    Mod::get()->setSavedValue<int>("diamonds", ZenGardenLayer::m_diamondCount);
    Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
    CCDirector::sharedDirector()->popScene();
}

void ZenGardenLayer::keyBackClicked()
{
    onBack(nullptr);
}

void ZenGardenLayer::onFeedOrbs(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 1)
    {
        ZenGardenLayer::m_selectedItem = 1;
        ZenGardenLayer::m_orbsHoverSprite->setVisible(true);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
    else
    {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedStars(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 2 && ZenGardenLayer::m_starCount > 0)
    {
        ZenGardenLayer::m_selectedItem = 2;
        ZenGardenLayer::m_starsHoverSprite->setVisible(true);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
    else
    {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedMoons(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 3 && ZenGardenLayer::m_moonCount > 0)
    {
        ZenGardenLayer::m_selectedItem = 3;
        ZenGardenLayer::m_moonsHoverSprite->setVisible(true);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
    else
    {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedDiamonds(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 4 && ZenGardenLayer::m_diamondCount > 0)
    {
        ZenGardenLayer::m_selectedItem = 4;
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(true);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    }
    else
    {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
}

bool ZenGardenLayer::init()
{
    if (!CCLayer::init())
        return false;

    // Automatically set the shards to the new value
    int getShards = GameStatsManager::sharedState()->getStat("29");
    ZenGardenLayer::m_diamondShards = Mod::get()->setSavedValue<int>("money", getShards);


    this->scheduleUpdate();
    
    // Enable keyboard input for cheat functionality
    this->setKeyboardEnabled(true);

    this->setID("zen-garden");

    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190};

    ZenGardenLayer::m_starCount = Mod::get()->getSavedValue<int>("stars");
    ZenGardenLayer::m_moonCount = Mod::get()->getSavedValue<int>("moons");
    ZenGardenLayer::m_diamondCount = Mod::get()->getSavedValue<int>("diamonds");
    ZenGardenLayer::m_diamondShards = Mod::get()->getSavedValue<int>("money");

    // Initialize maturity system variables
    ZenGardenLayer::m_maturityLevel = Mod::get()->getSavedValue<int>("player_maturity", 0);
    ZenGardenLayer::m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed", 0);
    ZenGardenLayer::m_lastOrbFeedTime = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed", 0);

    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    auto background = createLayerBG();

    this->addChild(background, -5);

    auto exitMenu = CCMenu::create();
    exitMenu->setID("exit-menu");

    this->addChild(exitMenu);

    auto close = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(ZenGardenLayer::onBack));

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
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedOrbs));

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
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedStars));

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
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedMoons));

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
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedDiamonds));

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

    ZenGardenLayer::m_diamondShardsLabel = CCLabelBMFont::create("-", "bigFont.fnt");
    ZenGardenLayer::m_diamondShardsLabel->setScale(0.65f);
    ZenGardenLayer::m_diamondShardsLabel->setID("money-count");
    ZenGardenLayer::m_diamondShardsLabel->setPosition(windowSize.width / 2, windowSize.height - 300);
    
    // diamond icon next to the label
    auto diamondIcon = CCSprite::createWithSpriteFrameName("currencyDiamondIcon_001.png");
    diamondIcon->setScale(0.8f);
    diamondIcon->setID("diamond-currency-icon");
    
    float labelWidth = m_diamondShardsLabel->getContentWidth() * m_diamondShardsLabel->getScale();
    diamondIcon->setPosition(ccp(m_diamondShardsLabel->getPositionX() - (labelWidth / 2) - 15, m_diamondShardsLabel->getPositionY()));

    this->addChild(diamondIcon);
    this->addChild(ZenGardenLayer::m_diamondShardsLabel);
    
    // Store the diamond icon as a member variable to update its position later
    m_diamondCurrencyIcon = diamondIcon;

    auto iconBackgrounds = CCSpriteBatchNode::create("GJ_button_05.png");
    iconBackgrounds->setZOrder(-1);
    iconBackgrounds->setID("icon-backgrounds");

    int x;
    int y;
    CCSprite *iconBackgroundSprite;
    int test;

    for (int i = 0; i < 32; i++)
    {
        x = xPositions[i % 8];

        if (i < 8)
        {
            y = 75;
        }
        else if (i < 16)
        {
            y = 25;
        }
        else if (i < 24)
        {
            y = -25;
        }
        else if (i < 32)
        {
            y = -75;
        }

        x = (windowSize.width / 2) + x;
        y = (windowSize.height / 2) + y;

        iconBackgroundSprite = CCSprite::create("GJ_button_05.png");
        iconBackgroundSprite->setPosition(ccp(x, y));

        iconBackgrounds->addChild(iconBackgroundSprite);
    }

    this->addChild(iconBackgrounds);

    // Create a SimplePlayer and place it in the first box
    ZenGardenLayer::m_simplePlayer = SimplePlayer::create(1); // Create with cube icon
    if (ZenGardenLayer::m_simplePlayer)
    {
        // horrible position
        int firstBoxX = (windowSize.width / 2) + xPositions[0]; // -190 offset
        int firstBoxY = (windowSize.height / 2) + 75;           // Top row

        // Set scale based on maturity level (0.5 for baby, growing to 0.8 at max level)
        float playerScale = 0.5f + (ZenGardenLayer::m_maturityLevel * 0.06f);
        playerScale = std::min(playerScale, 0.8f); // Cap at 0.8f

        ZenGardenLayer::m_simplePlayer->setScale(playerScale);
        ZenGardenLayer::m_simplePlayer->setID("simple-player-1");
        ZenGardenLayer::m_simplePlayer->setZOrder(-1);

        // click the simple player
        auto simplePlayerButton = CCMenuItemSpriteExtra::create(
            ZenGardenLayer::m_simplePlayer,
            this,
            menu_selector(ZenGardenLayer::onSimplePlayerClicked));

        simplePlayerButton->setID("simple-player-button");
        simplePlayerButton->setContentSize({40.f, 40.f});
        ZenGardenLayer::m_simplePlayer->setPosition({simplePlayerButton->getContentSize().width / 2, simplePlayerButton->getContentSize().height / 2});

        // menu for the SimplePlayer
        auto simplePlayerMenu = CCMenu::create();
        simplePlayerMenu->addChild(simplePlayerButton);
        simplePlayerMenu->setPosition(ccp(firstBoxX, firstBoxY));
        simplePlayerMenu->setID("simple-player-menu");

        this->addChild(simplePlayerMenu);

        // Add requirement indicator
        displayRequirementSprite();
    }

    // Create reset progress button at the bottom left of the screen using ButtonSprite
    auto resetButtonMenu = CCMenu::create();
    resetButtonMenu->setID("reset-button-menu");

    // Use ButtonSprite to create the reset button with better visual styling
    auto resetButtonSprite = ButtonSprite::create("Reset", "goldFont.fnt", "GJ_button_06.png", 0.8f);

    auto resetButton = CCMenuItemSpriteExtra::create(
        resetButtonSprite,
        this,
        menu_selector(ZenGardenLayer::onResetProgress));

    resetButton->setID("reset-progress-button");
    resetButtonMenu->addChild(resetButton);

    // Position at the bottom left corner
    resetButtonMenu->setPosition(ccp(55, 25));

    this->addChild(resetButtonMenu);

    return true;
}

void ZenGardenLayer::onSimplePlayerClicked(CCObject *sender)
{
    // Check if any item is selected (m_selectedItem != 0)
    if (ZenGardenLayer::m_selectedItem != 0)
    {
        bool validFeed = false;

        // Handle feeding based on maturity level and selected item
        if (m_maturityLevel < 1 && ZenGardenLayer::m_selectedItem == 1)
        {
            // Baby needs orbs, check for cooldown
            if (canFeedOrb())
            {
                m_orbsFeeded++;
                Mod::get()->setSavedValue<int>("player_orbs_fed", m_orbsFeeded);

                // Update last feed time using system time in seconds
                auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();
                m_lastOrbFeedTime = currentTime;
                Mod::get()->setSavedValue<int64_t>("player_last_orb_feed", m_lastOrbFeedTime);

                // Check if we've reached 5 orbs to level up
                if (m_orbsFeeded >= 5)
                {
                    handlePlayerGrowth();
                }

                validFeed = true;
            }
            else
            {
                displayOrbCooldownMessage();
            }
        }
        else if (m_maturityLevel == 1 && ZenGardenLayer::m_selectedItem == 2)
        {
            // Young player needs stars
            if (ZenGardenLayer::m_starCount > 0)
            {
                ZenGardenLayer::m_starCount -= 1;
                Mod::get()->setSavedValue<int>("stars", ZenGardenLayer::m_starCount);
                handlePlayerGrowth();
                validFeed = true;
            }
            else
            {
                log::debug("Not Enough Stars");
                Notification::create("Not Enough Stars", NotificationIcon::Warning, 0.5f)->show();
            }
        }
        else if (m_maturityLevel == 2 && ZenGardenLayer::m_selectedItem == 3)
        {
            // Teen player needs moons
            if (ZenGardenLayer::m_moonCount > 0)
            {
                ZenGardenLayer::m_moonCount -= 1;
                Mod::get()->setSavedValue<int>("moons", ZenGardenLayer::m_moonCount);
                handlePlayerGrowth();
                validFeed = true;
            }
            else
            {
                log::debug("Not Enough Moons");
                Notification::create("Not Enough Moons", NotificationIcon::Warning, 0.5f)->show();
            }
        }
        else if (m_maturityLevel >= 3 && m_maturityLevel < 5 && ZenGardenLayer::m_selectedItem == 4)
        {
            // Adult and elder players need diamonds
            if (ZenGardenLayer::m_diamondCount > 0)
            {
                ZenGardenLayer::m_diamondCount -= 1;
                Mod::get()->setSavedValue<int>("diamonds", ZenGardenLayer::m_diamondCount);
                handlePlayerGrowth();
                validFeed = true;
            }
            else
            {
                Notification::create("Not Enough Diamonds", NotificationIcon::Warning, 0.5f)->show();
            }
        }
        else
        {
            // Wrong food for current maturity level
            std::string message;
            if (m_maturityLevel < 1)
            {
                message = "Your baby player needs orbs to grow.";
            }
            else if (m_maturityLevel == 1)
            {
                message = "Your young player needs stars to grow.";
            }
            else if (m_maturityLevel == 2)
            {
                message = "Your teenage player needs moons to grow.";
            }
            else if (m_maturityLevel < 5)
            {
                message = "Your adult player needs diamonds to grow.";
            }
            else
            {
                message = "Your player is fully mature and doesn't need to eat.";
            }

            log::debug("Wrong Food: {}", message);
            Notification::create("Wrong Food: " + message, NotificationIcon::Warning, 0.5f)->show();
        }

        // If feed was valid, check for coin rewards
        if (validFeed)
        {
            tryEmitCoins();
            Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
            flashShards();

            // Update the requirement visuals
            displayRequirementSprite();
        }

        // Reset selected item after feeding attempt
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
    else
    {
        // No item selected, show player info popup
        auto playerInfo = SimplePlayerInfo::create(ZenGardenLayer::m_simplePlayer);
        if (playerInfo)
        {
            playerInfo->show();
        }
    }
}
void ZenGardenLayer::showBronzeCoinReward()
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int coinX = (windowSize.width / 2) + xPositions[0];
    int coinY = (windowSize.height / 2) + 75 + 30; // 30 pixels above the player

    newCoinSprite->setPosition(ccp(coinX, coinY));
    newCoinSprite->setVisible(true);
    newCoinSprite->setOpacity(255);
    newCoinSprite->setScale(0.5f);
    newCoinSprite->setColor({121, 75, 42}); // Brown color

    this->addChild(newCoinSprite);

    // Create a fade out and move up animation
    auto fadeOut = CCFadeOut::create(2.0f);
    auto moveUp = CCMoveBy::create(2.0f, ccp(0, 50));
    auto scaleUp = CCScaleTo::create(0.2f, 0.7f);
    auto scaleDown = CCScaleTo::create(1.8f, 0.3f);

    auto scaleSequence = CCSequence::create(scaleUp, scaleDown, nullptr);
    auto moveAndFade = CCSpawn::create(fadeOut, moveUp, scaleSequence, nullptr);

    // Remove the coin after animation using CCCallFuncN
    auto removeCallback = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto sequence = CCSequence::create(moveAndFade, removeCallback, nullptr);

    newCoinSprite->runAction(sequence);

    // Show text indicating +10
    auto rewardText = CCLabelBMFont::create("+10", "bigFont.fnt");
    rewardText->setScale(0.4f);
    rewardText->setPosition(ccp(coinX + 35, coinY));
    rewardText->setColor({121, 75, 42}); // Brown color
    this->addChild(rewardText);

    auto textFadeOut = CCFadeOut::create(1.8f);
    auto textMoveUp = CCMoveBy::create(1.8f, ccp(0, 35));
    auto textSpawn = CCSpawn::create(textFadeOut, textMoveUp, nullptr);
    auto textRemove = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto textSequence = CCSequence::create(textSpawn, textRemove, nullptr);

    rewardText->runAction(textSequence);
}

void ZenGardenLayer::showSilverCoinReward()
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int coinX = (windowSize.width / 2) + xPositions[0];
    int coinY = (windowSize.height / 2) + 75 + 30; // 30 pixels above the player

    newCoinSprite->setPosition(ccp(coinX, coinY));
    newCoinSprite->setVisible(true);
    newCoinSprite->setOpacity(255);
    newCoinSprite->setScale(0.6f); // Slightly larger than regular

    this->addChild(newCoinSprite);

    // Create a fade out and move up animation with sparkle effect
    auto fadeOut = CCFadeOut::create(2.5f);
    auto moveUp = CCMoveBy::create(2.5f, ccp(0, 60));
    auto scaleUp = CCScaleTo::create(0.3f, 0.8f);
    auto scaleDown = CCScaleTo::create(2.2f, 0.4f);

    auto scaleSequence = CCSequence::create(scaleUp, scaleDown, nullptr);
    auto moveAndFade = CCSpawn::create(fadeOut, moveUp, scaleSequence, nullptr);

    // Remove the coin after animation
    auto removeCallback = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto sequence = CCSequence::create(moveAndFade, removeCallback, nullptr);

    newCoinSprite->runAction(sequence);

    // Show text indicating +100
    auto rewardText = CCLabelBMFont::create("+100", "bigFont.fnt");
    rewardText->setScale(0.5f);
    rewardText->setPosition(ccp(coinX + 35, coinY));
    this->addChild(rewardText);

    auto textFadeOut = CCFadeOut::create(2.0f);
    auto textMoveUp = CCMoveBy::create(2.0f, ccp(0, 40));
    auto textSpawn = CCSpawn::create(textFadeOut, textMoveUp, nullptr);
    auto textRemove = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto textSequence = CCSequence::create(textSpawn, textRemove, nullptr);

    rewardText->runAction(textSequence);
}

void ZenGardenLayer::showGoldCoinReward()
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int coinX = (windowSize.width / 2) + xPositions[0];
    int coinY = (windowSize.height / 2) + 75 + 30; // 30 pixels above the player

    newCoinSprite->setPosition(ccp(coinX, coinY));
    newCoinSprite->setVisible(true);
    newCoinSprite->setOpacity(255);
    newCoinSprite->setScale(0.7f);          // Larger than silver
    newCoinSprite->setColor({255, 215, 0}); // Gold color

    this->addChild(newCoinSprite);

    // Create a fancy animation for gold coins
    auto fadeOut = CCFadeOut::create(3.0f);
    auto moveUp = CCMoveBy::create(3.0f, ccp(0, 70));
    auto scaleUp = CCScaleTo::create(0.4f, 1.0f);
    auto scaleDown = CCScaleTo::create(2.6f, 0.5f);

    auto scaleSequence = CCSequence::create(scaleUp, scaleDown, nullptr);
    auto moveAndFade = CCSpawn::create(fadeOut, moveUp, scaleSequence, nullptr);

    // Remove the coin after animation
    auto removeCallback = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto sequence = CCSequence::create(moveAndFade, removeCallback, nullptr);

    newCoinSprite->runAction(sequence);

    // Show text indicating +1000
    auto rewardText = CCLabelBMFont::create("+1000", "bigFont.fnt");
    rewardText->setScale(0.6f);
    rewardText->setPosition(ccp(coinX + 35, coinY));
    rewardText->setColor({255, 215, 0}); // Gold color
    this->addChild(rewardText);

    auto textFadeOut = CCFadeOut::create(2.5f);
    auto textMoveUp = CCMoveBy::create(2.5f, ccp(0, 50));
    auto textSpawn = CCSpawn::create(textFadeOut, textMoveUp, nullptr);
    auto textRemove = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::removeCoinSprite));
    auto textSequence = CCSequence::create(textSpawn, textRemove, nullptr);

    rewardText->runAction(textSequence);
}

void ZenGardenLayer::tryEmitCoins()
{
    // Generate random number between 1-100
    int randomChance = rand() % 100 + 1;

    // 90% chance for bronze coin
    if (randomChance <= 90)
    {
        showBronzeCoinReward();
        ZenGardenLayer::m_diamondShards += 10;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
    }

    // 9% chance for silver coin (91-99)
    if (randomChance >= 91 && randomChance <= 99)
    {
        showSilverCoinReward();
        ZenGardenLayer::m_diamondShards += 100;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
        flashShards();
    }
    // 1% chance for gold coin (100)
    else if (randomChance == 100)
    {
        showGoldCoinReward();
        ZenGardenLayer::m_diamondShards += 1000;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
        flashShards();
    }
}

void ZenGardenLayer::flashShards()
{
    if (!ZenGardenLayer::m_diamondShardsLabel)
        return;

    ZenGardenLayer::m_diamondShardsLabel->stopAllActions();
    ZenGardenLayer::m_diamondShardsLabel->setColor({ 0, 225, 255 });
    auto fadeToWhite = CCTintTo::create(2.0f, 255, 255, 255);

    ZenGardenLayer::m_diamondShardsLabel->runAction(fadeToWhite);
}

void ZenGardenLayer::removeCoinSprite(CCNode *sender)
{
    if (sender && sender->getParent())
    {
        sender->removeFromParent();
    }
}

void ZenGardenLayer::displayRequirementSprite()
{
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190};
    int iconX = (windowSize.width / 2) + xPositions[0] - 15; // Position to the left of player
    int iconY = (windowSize.height / 2) + 75 + 15;           // Position above player

    // Remove old sprite and label if they exist
    if (m_requirementSprite)
    {
        m_requirementSprite->removeFromParent();
        m_requirementSprite = nullptr;
    }

    if (m_requirementLabel)
    {
        m_requirementLabel->removeFromParent();
        m_requirementLabel = nullptr;
    }

    // Don't show anything if fully mature (level 5)
    if (m_maturityLevel >= 5)
    {
        return;
    }

    // Create appropriate sprite based on maturity level
    if (m_maturityLevel < 1)
    {
        m_requirementSprite = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
        m_requirementSprite->setColor({255, 255, 255});

        // Add orb counter label
        m_requirementLabel = CCLabelBMFont::create((std::to_string(m_orbsFeeded) + "/5").c_str(), "goldFont.fnt");
        m_requirementLabel->setScale(0.3f);
        m_requirementLabel->setPosition(ccp(iconX + 12, iconY - 8));
        this->addChild(m_requirementLabel);
    }
    else if (m_maturityLevel < 2)
    {
        m_requirementSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    }
    else if (m_maturityLevel < 3)
    {
        m_requirementSprite = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    }
    else if (m_maturityLevel < 5)
    {
        m_requirementSprite = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
    }

    if (m_requirementSprite)
    {
        m_requirementSprite->setScale(0.4f);
        m_requirementSprite->setPosition(ccp(iconX, iconY));
        this->addChild(m_requirementSprite);
    }
}

void ZenGardenLayer::hideRequirementSprite()
{
    if (m_requirementSprite)
    {
        m_requirementSprite->removeFromParent();
        m_requirementSprite = nullptr;
    }

    if (m_requirementLabel)
    {
        m_requirementLabel->removeFromParent();
        m_requirementLabel = nullptr;
    }
}

void ZenGardenLayer::updatePlayerMaturityVisuals()
{
    // Update player scale based on maturity level
    if (m_simplePlayer)
    {
        float playerScale = 0.5f + (m_maturityLevel * 0.06f);
        playerScale = std::min(playerScale, 0.8f); // Cap at 0.8f
        m_simplePlayer->setScale(playerScale);
    }

    // Update requirement sprite
    displayRequirementSprite();
}

void ZenGardenLayer::handlePlayerGrowth()
{
    // Handle growth based on maturity level
    if (m_maturityLevel < 5)
    {
        // Increase maturity level
        m_maturityLevel++;

        // Reset orbs fed counter when leveling up from baby
        if (m_maturityLevel == 1)
        {
            m_orbsFeeded = 0;
        }

        // Save updated maturity level
        Mod::get()->setSavedValue<int>("player_maturity", m_maturityLevel);

        // Update visuals
        updatePlayerMaturityVisuals();

        // Show message
        std::string message = "Your player grew to maturity level " + std::to_string(m_maturityLevel) + "!";
        FLAlertLayer::create(
            "Level Up!",
            message,
            "OK")
            ->show();
    }
}

bool ZenGardenLayer::canFeedOrb()
{
    // system time
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    int64_t timeSinceLastFeed = currentTime - m_lastOrbFeedTime;

    log::debug("Time since last feed: {} seconds", timeSinceLastFeed);

    // 15-20 second cooldown (use 17.5 seconds)
    return timeSinceLastFeed >= 17;
}

void ZenGardenLayer::displayOrbCooldownMessage()
{
    // accurate seconds calculation
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    int64_t timeSinceLastFeed = currentTime - m_lastOrbFeedTime;
    int64_t remainingTime = 17 - timeSinceLastFeed;

    // don't show negative time
    int secondsRemaining = static_cast<int>(remainingTime > 0 ? remainingTime : 0);

    std::string message = "Please wait " + std::to_string(secondsRemaining) + " seconds.";

    log::debug("Feeding Cooldown: {}", message);
    Notification::create("Feeding Cooldown: " + message, NotificationIcon::Loading, 0.5f)->show();
}

void ZenGardenLayer::update(float dt)
{
    if (ZenGardenLayer::m_selectedItem != 0)
    {
        switch (m_selectedItem)
        {
        case 1:
            ZenGardenLayer::m_orbsHoverSprite->setPosition(getMousePos());
            break;
        case 2:
            ZenGardenLayer::m_starsHoverSprite->setPosition(getMousePos());
            break;
        case 3:
            ZenGardenLayer::m_moonsHoverSprite->setPosition(getMousePos());
            break;
        case 4:
            ZenGardenLayer::m_diamondsHoverSprite->setPosition(getMousePos());
            break;
        }
    }

    ZenGardenLayer::m_starsLabel->setString(std::to_string(ZenGardenLayer::m_starCount).c_str(), true);
    ZenGardenLayer::m_moonsLabel->setString(std::to_string(ZenGardenLayer::m_moonCount).c_str(), true);
    ZenGardenLayer::m_diamondsLabel->setString(std::to_string(ZenGardenLayer::m_diamondCount).c_str(), true);
    ZenGardenLayer::m_diamondShardsLabel->setString((std::to_string(ZenGardenLayer::m_diamondShards)).c_str(), true);
    
    // Update diamond currency icon position based on label width
    if (m_diamondCurrencyIcon && m_diamondShardsLabel) {
        float labelWidth = m_diamondShardsLabel->getContentWidth() * m_diamondShardsLabel->getScale();
        m_diamondCurrencyIcon->setPosition(ccp(m_diamondShardsLabel->getPositionX() - (labelWidth / 2) - 15, m_diamondShardsLabel->getPositionY()));
    }

    // If we have a requirement label showing for orbs (maturity level 0), update it
    if (m_maturityLevel == 0 && m_requirementLabel)
    {
        m_requirementLabel->setString((std::to_string(m_orbsFeeded) + "/5").c_str(), true);
    }
}

void ZenGardenLayer::onResetProgress(CCObject *sender)
{
    if (m_diamondShards >= 1000) {
    // Show confirmation dialog using Geode's createQuickPopup
    geode::createQuickPopup(
        "Reset Progress",
        "Are you sure you want to reset <cr>ALL</c> progress?\n\nThis will reset your maturity level, resources, and all player stats.\nThis will cost <cc>1000 Diamond Shards</c> to reset\n<cr>This action cannot be undone!</c>",
        "Cancel", "Reset",
        [this](auto, bool btn2)
        {
            if (btn2)
            {
                m_diamondShards -= 1000;
                GameStatsManager::sharedState()->setStat("29", m_diamondShards);
                this->confirmResetProgress();
            }
        });
    } else {
        Notification::create("You don't have enough to reset! Required 1000 Diamond Shards", NotificationIcon::Error, 0.5f)->show();
    }
}

void ZenGardenLayer::confirmResetProgress()
{
    // Reset all saved values to default
    Mod::get()->setSavedValue<int>("player_maturity", 0);
    Mod::get()->setSavedValue<int>("player_orbs_fed", 0);
    Mod::get()->setSavedValue<float>("player_last_orb_feed", 0.0f);
    Mod::get()->setSavedValue<int>("stars", 2);
    Mod::get()->setSavedValue<int>("moons", 3);
    Mod::get()->setSavedValue<int>("diamonds", 4);
    Mod::get()->setSavedValue<int>("money", 0);

    // Update in-memory values
    m_maturityLevel = 0;
    m_orbsFeeded = 0;
    m_lastOrbFeedTime = 0;
    m_starCount = 2;
    m_moonCount = 3;
    m_diamondCount = 4;
    m_diamondShards = GameStatsManager::sharedState()->getStat("29");

    // Update player scale based on new maturity level
    if (m_simplePlayer)
    {
        m_simplePlayer->setScale(0.5f); // Base scale for baby player
    }

    // Update visuals
    displayRequirementSprite();

    // Show confirmation message
    FLAlertLayer::create(
        "Progress Reset",
        "Your progress has been reset to default values!",
        "OK")
        ->show();
}


// I accidentally wipped my diamond shards so this is needed for me xD
// also using this is considered cheating
void ZenGardenLayer::cheat() {

    ZenGardenLayer::m_diamondShards += 1000;
    GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
    Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
    flashShards();
}

void ZenGardenLayer::keyDown(enumKeyCodes key) {
    // Check for the "J" key press (74 is the key code for 'J')
    if (key == KEY_J) {
        cheat();
    }
    
    CCLayer::keyDown(key);
}