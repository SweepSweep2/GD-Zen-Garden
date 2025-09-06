#include "ZenGardenLayer.hpp"
#include "ZenGardenShopLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/ui/Notification.hpp>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <sstream>

// Static member definitions
int ZenGardenLayer::m_starCount = 0;
int ZenGardenLayer::m_moonCount = 0;
int ZenGardenLayer::m_diamondCount = 0;
int ZenGardenLayer::m_diamondShards = 0;
CCLabelBMFont *ZenGardenLayer::m_starsLabel = nullptr;
CCLabelBMFont *ZenGardenLayer::m_moonsLabel = nullptr;
CCLabelBMFont *ZenGardenLayer::m_diamondsLabel = nullptr;
CCLabelBMFont *ZenGardenLayer::m_diamondShardsLabel = nullptr;

ZenGardenLayer *ZenGardenLayer::create()
{
    auto ret = new ZenGardenLayer();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}
bool ZenGardenLayer::init()
{
    if (!CCLayer::init())
        return false;

    // Automatically set the shards to the new value
    int getShards = GameStatsManager::sharedState()->getStat("29");
    ZenGardenLayer::m_diamondShards = Mod::get()->setSavedValue<int>("money", getShards);

    this->scheduleUpdate();

    this->setKeyboardEnabled(true);

    this->setID("zen-garden");

    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190};

    ZenGardenLayer::m_starCount = Mod::get()->getSavedValue<int>("stars", -1);
    ZenGardenLayer::m_moonCount = Mod::get()->getSavedValue<int>("moons", -1);
    ZenGardenLayer::m_diamondCount = Mod::get()->getSavedValue<int>("diamonds", -1);
    ZenGardenLayer::m_diamondShards = Mod::get()->getSavedValue<int>("money");

    if (ZenGardenLayer::m_starCount < 0) {
        ZenGardenLayer::m_starCount = 5;
    }
    if (ZenGardenLayer::m_moonCount < 0) {
        ZenGardenLayer::m_moonCount = 5;
    }
    if (ZenGardenLayer::m_diamondCount < 0) {
        ZenGardenLayer::m_diamondCount = 5;
    }

    // Initialize maturity system variables (used for the currently selected player)
    ZenGardenLayer::m_maturityLevel = 0;
    ZenGardenLayer::m_orbsFeeded = 0;
    ZenGardenLayer::m_lastOrbFeedTime = 0;

    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    auto background = createLayerBG();
    this->addChild(background, -5);

    auto exitMenu = CCMenu::create();
    exitMenu->setID("exit-menu");
    exitMenu->setPosition(ccp(windowSize.width / 2 - 50, windowSize.height / 2));

    this->addChild(exitMenu);

    auto close = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(ZenGardenLayer::onBack));

    close->setPosition((windowSize.width / 2), (windowSize.height / 2) - 32);
    close->setID("exit-zen-garden");

    exitMenu->addChild(close);

    auto toShop = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("shopRope_001.png"),
        this,
        menu_selector(ZenGardenLayer::onShop));

    toShop->setPosition((windowSize.width / 2) - 120, (windowSize.height / 2) - 10);
    toShop->setID("shop-button");
    exitMenu->addChild(toShop);

    auto topBarMenu = CCMenu::create();
    topBarMenu->setID("top-bar-menu");
    this->addChild(topBarMenu);

    auto topBarBG = CCScale9Sprite::create("square02b_001.png");
    topBarBG->setContentSize({163, 40});
    topBarBG->setPosition((windowSize.width / -2) + 124.5, (windowSize.height / 2) - 25);
    topBarBG->setID("top-bar-bg");
    topBarBG->setColor({0, 36, 91});
    topBarMenu->addChild(topBarBG);

    ZenGardenLayer::m_feedOrbsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "currencyOrbIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedOrbs));
    ZenGardenLayer::m_feedOrbsButton->setID("feed-orbs");
    ZenGardenLayer::m_feedOrbsButton->setPosition((windowSize.width / -2) + 65.5, (windowSize.height / 2) - 25);
    topBarMenu->addChild(ZenGardenLayer::m_feedOrbsButton);

    ZenGardenLayer::m_orbsHoverSprite = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
    ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    ZenGardenLayer::m_orbsHoverSprite->setID("orbs-hover");
    ZenGardenLayer::m_orbsHoverSprite->setZOrder(10);
    this->addChild(ZenGardenLayer::m_orbsHoverSprite);

    auto orbCount = CCLabelBMFont::create("8", "bigFont.fnt");
    orbCount->setID("orb-count");
    orbCount->setPosition((windowSize.width / -2) + 75.5, (windowSize.height / 2) - 34);
    orbCount->setRotation(90.0f);
    orbCount->setScale(0.5f);
    topBarMenu->addChild(orbCount);

    ZenGardenLayer::m_feedStarsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_starsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedStars));
    ZenGardenLayer::m_feedStarsButton->setID("feed-stars");
    ZenGardenLayer::m_feedStarsButton->setPosition((windowSize.width / -2) + 105, (windowSize.height / 2) - 25);
    topBarMenu->addChild(ZenGardenLayer::m_feedStarsButton);

    ZenGardenLayer::m_starsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    ZenGardenLayer::m_starsHoverSprite->setVisible(false);
    ZenGardenLayer::m_starsHoverSprite->setID("stars-hover");
    ZenGardenLayer::m_starsHoverSprite->setZOrder(10);
    this->addChild(ZenGardenLayer::m_starsHoverSprite);

    ZenGardenLayer::m_starsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_starCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_starsLabel->setID("star-count");
    ZenGardenLayer::m_starsLabel->setPosition((windowSize.width / -2) + 115, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_starsLabel->setScale(0.5f);
    topBarMenu->addChild(ZenGardenLayer::m_starsLabel);

    ZenGardenLayer::m_feedMoonsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_moonsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedMoons));
    ZenGardenLayer::m_feedMoonsButton->setID("feed-moons");
    ZenGardenLayer::m_feedMoonsButton->setPosition((windowSize.width / -2) + 144.5, (windowSize.height / 2) - 25);
    topBarMenu->addChild(ZenGardenLayer::m_feedMoonsButton);

    ZenGardenLayer::m_moonsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
    ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
    ZenGardenLayer::m_moonsHoverSprite->setID("moons-hover");
    ZenGardenLayer::m_moonsHoverSprite->setZOrder(10);
    this->addChild(ZenGardenLayer::m_moonsHoverSprite);

    ZenGardenLayer::m_moonsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_moonCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_moonsLabel->setID("moon-count");
    ZenGardenLayer::m_moonsLabel->setPosition((windowSize.width / -2) + 154.5, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_moonsLabel->setScale(0.5f);
    topBarMenu->addChild(ZenGardenLayer::m_moonsLabel);

    ZenGardenLayer::m_feedDiamondsButton = CCMenuItemSpriteExtra::create(
        geode::EditorButtonSprite::createWithSpriteFrameName(
            "GJ_diamondsIcon_001.png",
            1.0f,
            geode::EditorBaseColor::Green,
            {}),
        this,
        menu_selector(ZenGardenLayer::onFeedDiamonds));
    ZenGardenLayer::m_feedDiamondsButton->setID("feed-diamonds");
    ZenGardenLayer::m_feedDiamondsButton->setPosition((windowSize.width / -2) + 184, (windowSize.height / 2) - 25);
    topBarMenu->addChild(ZenGardenLayer::m_feedDiamondsButton);

    // Move Mode toggle button
    auto moveModeButton = CCMenuItemSpriteExtra::create(
        EditorButtonSprite::createWithSpriteFrameName(
            "GJ_chatBtn_02_001.png",
            1.0f,
            EditorBaseColor::Salmon,
            {}),
        this,
        menu_selector(ZenGardenLayer::onToggleMoveMode));
    moveModeButton->setID("move-mode-button");
    moveModeButton->setPosition((windowSize.width / -2) + 230.5f, (windowSize.height / 2) - 25);
    topBarMenu->addChild(moveModeButton);

    ZenGardenLayer::m_diamondsHoverSprite = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
    ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    ZenGardenLayer::m_diamondsHoverSprite->setID("diamonds-hover");
    ZenGardenLayer::m_diamondsHoverSprite->setZOrder(10);
    this->addChild(ZenGardenLayer::m_diamondsHoverSprite);

    ZenGardenLayer::m_diamondsLabel = CCLabelBMFont::create(std::to_string(ZenGardenLayer::m_diamondCount).c_str(), "bigFont.fnt");
    ZenGardenLayer::m_diamondsLabel->setID("diamond-count");
    ZenGardenLayer::m_diamondsLabel->setPosition((windowSize.width / -2) + 194, (windowSize.height / 2) - 34);
    ZenGardenLayer::m_diamondsLabel->setScale(0.5f);
    topBarMenu->addChild(ZenGardenLayer::m_diamondsLabel);

    ZenGardenLayer::m_diamondShardsLabel = CCLabelBMFont::create("-", "bigFont.fnt");
    ZenGardenLayer::m_diamondShardsLabel->setScale(0.65f);
    ZenGardenLayer::m_diamondShardsLabel->setID("money-count");
    ZenGardenLayer::m_diamondShardsLabel->setPosition(windowSize.width / 2, windowSize.height / 12);

    auto diamondIcon = CCSprite::createWithSpriteFrameName("currencyDiamondIcon_001.png");
    diamondIcon->setScale(0.8f);
    diamondIcon->setID("diamond-currency-icon");
    float labelWidth = m_diamondShardsLabel->getContentWidth() * m_diamondShardsLabel->getScale();
    diamondIcon->setPosition(ccp(m_diamondShardsLabel->getPositionX() - (labelWidth / 2) - 15, m_diamondShardsLabel->getPositionY()));
    this->addChild(diamondIcon);
    this->addChild(ZenGardenLayer::m_diamondShardsLabel);
    m_diamondCurrencyIcon = diamondIcon;

    // Move Mode label (top center), hidden by default
    m_moveModeLabel = CCLabelBMFont::create("Move Mode", "goldFont.fnt");
    if (m_moveModeLabel)
    {
        m_moveModeLabel->setID("move-mode-label");
        m_moveModeLabel->setPosition({windowSize.width / 2 + 20, windowSize.height - 20});
        m_moveModeLabel->setScale(0.7f);
        m_moveModeLabel->setVisible(false);
        m_moveModeLabel->setAlignment(kCCTextAlignmentCenter);
        this->addChild(m_moveModeLabel, 5);
    }

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

    // Clickable transparent buttons for each slot; enabled only when slot is empty.
    auto slotsMenu = CCMenu::create();
    slotsMenu->setPosition({0, 0});
    slotsMenu->setID("slots-menu");
    this->addChild(slotsMenu, 0); // below players-menu

    for (int i = 0; i < 32; i++)
    {
        int sx = xPositions[i % 8];
        int sy;
        if (i < 8)
            sy = 75;
        else if (i < 16)
            sy = 25;
        else if (i < 24)
            sy = -25;
        else
            sy = -75;

        sx = (windowSize.width / 2) + sx;
        sy = (windowSize.height / 2) + sy;

        auto slotSprite = CCSprite::create("GJ_button_05.png");
        if (slotSprite)
        {
            slotSprite->setOpacity(0); // invisible hitbox
            auto slotItem = CCMenuItemSpriteExtra::create(slotSprite, this, menu_selector(ZenGardenLayer::onSlotClicked));
            slotItem->setTag(i);
            slotItem->setPosition(ccp(sx, sy));
            slotsMenu->addChild(slotItem);
        }
    }

    // Build menu of purchased players
    auto playersMenu = CCMenu::create();
    playersMenu->setPosition({0, 0});
    playersMenu->setID("players-menu");
    this->addChild(playersMenu);

    // Ensure slot hitboxes reflect occupancy
    refreshSlotsMenuInteractable();

    // Load all saved SimplePlayers; if none, leave the garden empty
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    bool firstPlayerAssigned = false;
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;
        while (std::getline(ss, token, ','))
        {
            int pos = -1;
            if (!token.empty())
            {
                char *endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0')
                    pos = static_cast<int>(v);
            }
            if (pos < 0 || pos >= 32)
                continue;

            std::string key = "player_" + std::to_string(pos);
            std::string playerInfo = Mod::get()->getSavedValue<std::string>(key, "");
            if (playerInfo.empty())
                continue;

            // icon,color1,color2[,glow]
            std::stringstream playerSS(playerInfo);
            std::string infoToken;
            std::vector<int> values;
            while (std::getline(playerSS, infoToken, ','))
            {
                if (!infoToken.empty())
                {
                    char *endp = nullptr;
                    long v = std::strtol(infoToken.c_str(), &endp, 10);
                    if (endp && *endp == '\0')
                        values.push_back(static_cast<int>(v));
                }
            }
            if (values.size() < 3)
                continue;

            int iconId = values[0];
            int color1Idx = values[1];
            int color2Idx = values[2];
            bool glow = (values.size() >= 4 ? values[3] != 0 : false);

            // Create player and set frame
            auto playerNode = SimplePlayer::create(1);
            playerNode->updatePlayerFrame(iconId, static_cast<IconType>(0));
            // Store iconId on the node so popups can mirror it reliably
            playerNode->setTag(iconId);

            // Apply colors via GameManager indices
            auto gm = GameManager::sharedState();
            ccColor3B c1 = gm->colorForIdx(color1Idx);
            ccColor3B c2 = gm->colorForIdx(color2Idx);
            playerNode->setColors(c1, c2);

            // Position across 4 rows x 8 columns
            int col = pos % 8;
            int row = pos / 8; // 0..3
            int rowOffsets[4] = {75, 25, -25, -75};
            int playerX = (windowSize.width / 2) + xPositions[col];
            int playerY = (windowSize.height / 2) + rowOffsets[(row < 4 ? row : 3)];

            // Scale by per-player maturity level (default 0)
            int perMaturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
            float playerScale = std::min(0.5f + (perMaturity * 0.06f), 0.8f);

            // Create clickable item
            auto playerItem = CCMenuItemSpriteExtra::create(
                playerNode,
                this,
                menu_selector(ZenGardenLayer::onSimplePlayerClicked));
            playerItem->setID("purchased-player-" + std::to_string(pos));
            playerItem->setTag(pos);
            playerItem->setPosition(ccp(playerX, playerY));
            playerNode->setScale(playerScale);
            playerItem->setContentSize({40.f, 40.f});
            playerNode->setAnchorPoint(ccp(0.5f, 0.5f));
            playerNode->setPosition({playerItem->getContentSize().width / 2, playerItem->getContentSize().height / 2});
            playersMenu->addChild(playerItem);

            if (!firstPlayerAssigned)
            {
                ZenGardenLayer::m_simplePlayer = playerNode;
                m_activePos = pos;
                // Load the active player's maturity/orbs state
                ZenGardenLayer::m_maturityLevel = perMaturity;
                ZenGardenLayer::m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
                ZenGardenLayer::m_lastOrbFeedTime = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
                firstPlayerAssigned = true;
            }
        }
        if (firstPlayerAssigned)
        {
            displayRequirementSprite();
        }
    }

    // Create reset progress button at the bottom left of the screen using ButtonSprite
    auto resetButtonMenu = CCMenu::create();
    resetButtonMenu->setID("reset-button-menu");
    auto resetButtonSprite = ButtonSprite::create("Reset", "goldFont.fnt", "GJ_button_06.png", 0.8f);
    auto resetButton = CCMenuItemSpriteExtra::create(
        resetButtonSprite,
        this,
        menu_selector(ZenGardenLayer::onResetProgress));
    resetButton->setID("reset-progress-button");
    resetButtonMenu->addChild(resetButton);
    resetButtonMenu->setPosition(ccp(80, 25));
    this->addChild(resetButtonMenu);

    return true;
}

void ZenGardenLayer::onEnter()
{
    CCLayer::onEnter();
    // Rebuild the garden when re-entering the layer (e.g., after closing shop)
    // This keeps icons and requirement overlays in sync with saved state.
    // Note: function defined later in file.
    reloadGardenFromSaves();
    displayRequirementSprite();
}

void ZenGardenLayer::onSimplePlayerClicked(CCObject *sender)
{
    auto item = static_cast<CCNode *>(sender);
    int clickedPos = -1;
    if (auto mi = typeinfo_cast<CCMenuItem *>(item))
    {
        clickedPos = mi->getTag();
    }

    // In Move Mode, clicking a player picks it as the source
    if (m_moveMode && clickedPos >= 0)
    {
        // If another player was selected, stop its flashing and restore colors
        if (m_moveFlashPos >= 0 && m_moveFlashPos != clickedPos)
            stopMoveSelectionFlash();
        m_moveSourcePos = clickedPos;
        std::string playerName = Mod::get()->getSavedValue<std::string>("player_name_" + std::to_string(clickedPos), "");
        if (playerName.empty()) playerName = "Player";
        if (m_moveModeLabel)
            m_moveModeLabel->setString(("Move Mode\n(" + playerName + ")").c_str(), true);
        startMoveSelectionFlash(clickedPos);
        //Notification::create("Selected " + playerName + ". Choose an empty slot.", NotificationIcon::Info, 1.f)->show();
        return;
    }

    // If a feed item is selected, apply it to the clicked slot directly
    if (ZenGardenLayer::m_selectedItem != 0 && clickedPos >= 0)
    {
        bool validFeed = false;

        int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(clickedPos), 0);

        // Level 0: Needs 5 orbs; cooldown scales with level (base * 2^level)
        if (maturity < 1 && ZenGardenLayer::m_selectedItem == 1)
        {
            if (canFeedOrbForPos(clickedPos))
            {
                int orbsFed = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(clickedPos), 0) + 1;
                Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(clickedPos), orbsFed);

                auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();
                Mod::get()->setSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(clickedPos), currentTime);

                if (orbsFed >= 5)
                {
                    handlePlayerGrowthForPos(clickedPos);
                    maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(clickedPos), 0);
                }

                validFeed = true;
            }
            else
            {
                displayOrbCooldownMessageForPos(clickedPos);
            }
        }
        // Level 1: Needs 5 stars with cooldown
        else if (maturity == 1 && ZenGardenLayer::m_selectedItem == 2)
        {
            // Cooldown for stars doubles with level index (base 17 * 2^1)
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_star_feed_" + std::to_string(clickedPos), 0);
            int required = 17 * (1 << 1);
            if (currentTime - lastFeed >= required)
            {
                if (ZenGardenLayer::m_starCount > 0)
                {
                    ZenGardenLayer::m_starCount -= 1;
                    Mod::get()->setSavedValue<int>("stars", ZenGardenLayer::m_starCount);

                    int starsFed = Mod::get()->getSavedValue<int>("player_stars_fed_" + std::to_string(clickedPos), 0) + 1;
                    Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(clickedPos), starsFed);
                    Mod::get()->setSavedValue<int64_t>("player_last_star_feed_" + std::to_string(clickedPos), currentTime);

                    if (starsFed >= 5)
                    {
                        handlePlayerGrowthForPos(clickedPos);
                        maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(clickedPos), 0);
                    }

                    validFeed = true;
                }
                else
                {
                    Notification::create("Not Enough Stars", NotificationIcon::Warning, 1.f)->show();
                }
            }
            else
            {
                // Cooldown message for stars
                int remaining = required - static_cast<int>(currentTime - lastFeed);
                if (remaining < 0)
                    remaining = 0;
                std::string playerName = Mod::get()->getSavedValue<std::string>(
                    "player_name_" + std::to_string(clickedPos), "");
                if (playerName.empty())
                    playerName = "Player";
                log::debug("Feeding Cooldown (pos {}): {} seconds remaining", clickedPos, remaining);
                Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
            }
        }
        // Level 2: Needs 5 moons with cooldown
        else if (maturity == 2 && ZenGardenLayer::m_selectedItem == 3)
        {
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(clickedPos), 0);
            int required = 17 * (1 << 2);
            if (currentTime - lastFeed >= required)
            {
                if (ZenGardenLayer::m_moonCount > 0)
                {
                    ZenGardenLayer::m_moonCount -= 1;
                    Mod::get()->setSavedValue<int>("moons", ZenGardenLayer::m_moonCount);

                    int moonsFed = Mod::get()->getSavedValue<int>("player_moons_fed_" + std::to_string(clickedPos), 0) + 1;
                    Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(clickedPos), moonsFed);
                    Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(clickedPos), currentTime);

                    if (moonsFed >= 5)
                    {
                        handlePlayerGrowthForPos(clickedPos);
                        maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(clickedPos), 0);
                    }

                    validFeed = true;
                }
                else
                {
                    Notification::create("Not Enough Moons", NotificationIcon::Warning, 1.f)->show();
                }
            }
            else
            {
                int remaining = required - static_cast<int>(currentTime - lastFeed);
                if (remaining < 0)
                    remaining = 0;
                std::string playerName = Mod::get()->getSavedValue<std::string>(
                    "player_name_" + std::to_string(clickedPos), "");
                if (playerName.empty())
                    playerName = "Player";
                log::debug("Feeding Cooldown (pos {}): {} seconds remaining", clickedPos, remaining);
                Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
            }
        }
        // Level 5+: requires moons instead of diamonds, with cooldown; no further level-ups
        else if (maturity >= 5 && ZenGardenLayer::m_selectedItem == 3)
        {
            int levelForCooldown = 5; // cap
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(clickedPos), 0);
            int required = 17 * (1 << levelForCooldown);
            if (currentTime - lastFeed >= required)
            {
                if (ZenGardenLayer::m_moonCount > 0)
                {
                    ZenGardenLayer::m_moonCount -= 1;
                    Mod::get()->setSavedValue<int>("moons", ZenGardenLayer::m_moonCount);

                    int moonsFed = Mod::get()->getSavedValue<int>("player_moons_fed_" + std::to_string(clickedPos), 0) + 1;
                    Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(clickedPos), moonsFed);
                    Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(clickedPos), currentTime);

                    // No level-up beyond 5
                    validFeed = true;
                }
                else
                {
                    Notification::create("Not Enough Moons", NotificationIcon::Warning, 1.f)->show();
                }
            }
            else
            {
                int remaining = required - static_cast<int>(currentTime - lastFeed);
                if (remaining < 0)
                    remaining = 0;
                std::string playerName = Mod::get()->getSavedValue<std::string>(
                    "player_name_" + std::to_string(clickedPos), "");
                if (playerName.empty())
                    playerName = "Player";
                log::debug("Feeding Cooldown (pos {}): {} seconds remaining", clickedPos, remaining);
                Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
            }
        }
        // Level 3-4: Needs 5 diamonds per level with cooldown
        else if ((maturity == 3 || maturity == 4) && ZenGardenLayer::m_selectedItem == 4)
        {
            int levelForCooldown = maturity; // 3 or 4
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(clickedPos), 0);
            int required = 17 * (1 << levelForCooldown);
            if (currentTime - lastFeed >= required)
            {
                if (ZenGardenLayer::m_diamondCount > 0)
                {
                    ZenGardenLayer::m_diamondCount -= 1;
                    Mod::get()->setSavedValue<int>("diamonds", ZenGardenLayer::m_diamondCount);

                    // Track diamonds fed for levels 3 and 4; continue counting at 5+ but won't level up further
                    int diamondsFed = Mod::get()->getSavedValue<int>("player_diamonds_fed_" + std::to_string(clickedPos), 0) + 1;
                    Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(clickedPos), diamondsFed);
                    Mod::get()->setSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(clickedPos), currentTime);

                    if (maturity < 5 && diamondsFed >= 5)
                    {
                        handlePlayerGrowthForPos(clickedPos);
                        maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(clickedPos), 0);
                    }

                    validFeed = true;
                }
                else
                {
                    Notification::create("Not Enough Diamonds", NotificationIcon::Warning, 1.f)->show();
                }
            }
            else
            {
                int remaining = required - static_cast<int>(currentTime - lastFeed);
                if (remaining < 0)
                    remaining = 0;
                std::string playerName = Mod::get()->getSavedValue<std::string>(
                    "player_name_" + std::to_string(clickedPos), "");
                if (playerName.empty())
                    playerName = "Player";
                log::debug("Feeding Cooldown (pos {}): {} seconds remaining", clickedPos, remaining);
                Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
            }
        }
        else
        {
            std::string message;
            if (maturity < 1)
                message = "needs orbs.";
            else if (maturity == 1)
                message = "needs stars.";
            else if (maturity == 2)
                message = "needs moons.";
            else if (maturity == 3 || maturity == 4)
                message = "needs diamonds.";
            else
                message = "needs moons.";
            std::string playerName = Mod::get()->getSavedValue<std::string>(
                "player_name_" + std::to_string(clickedPos), "");
            if (playerName.empty())
                playerName = "Player";
            Notification::create(playerName + " " + message, NotificationIcon::Warning, 1.f)->show();
        }

        if (validFeed)
        {
            tryEmitCoins(clickedPos);
            Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
            flashShards();
            updatePlayerMaturityVisualsForPos(clickedPos);
            displayRequirementSprite();
        }

        // Always reset selection after an attempt
        ZenGardenLayer::m_selectedItem = 0;
        if (ZenGardenLayer::m_orbsHoverSprite)
            ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
            ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(255, 255, 255));
        if (ZenGardenLayer::m_starsHoverSprite)
            ZenGardenLayer::m_starsHoverSprite->setVisible(false);
            ZenGardenLayer::m_feedStarsButton->setColor(ccc3(255, 255, 255));
        if (ZenGardenLayer::m_moonsHoverSprite)
            ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
            ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(255, 255, 255));
        if (ZenGardenLayer::m_diamondsHoverSprite)
            ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
            ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(255, 255, 255));
        return;
    }

    // If no food is selected, keep previous behavior of updating active selection and showing info
    if (clickedPos >= 0)
    {
        m_activePos = clickedPos;
        SimplePlayer *clickedPlayer = nullptr;
        if (item->getChildren() && item->getChildren()->count() > 0)
        {
            clickedPlayer = typeinfo_cast<SimplePlayer *>(item->getChildren()->objectAtIndex(0));
        }
        if (clickedPlayer)
            m_simplePlayer = clickedPlayer;

        m_maturityLevel = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(m_activePos), 0);
        m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(m_activePos), 0);
        m_lastOrbFeedTime = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(m_activePos), 0);
        displayRequirementSprite();
    }

    if (ZenGardenLayer::m_simplePlayer)
    {
        int slotForPopup = m_activePos;
        if (auto playerInfo = SimplePlayerInfo::create(ZenGardenLayer::m_simplePlayer, slotForPopup))
        {
            playerInfo->show();
        }
    }
};

void ZenGardenLayer::reloadGardenFromSaves()
{
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190};

    // Remove existing requirement UI on all players
    hideRequirementSprite();

    // Remove existing players menu (if any)
    if (auto oldMenu = this->getChildByID("players-menu"))
    {
        this->removeChild(oldMenu);
    }

    // Build menu of purchased players again
    auto playersMenu = CCMenu::create();
    playersMenu->setPosition({0, 0});
    playersMenu->setID("players-menu");
    this->addChild(playersMenu);

    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    bool firstPlayerAssigned = false;
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;
        while (std::getline(ss, token, ','))
        {
            // Parse slot index
            int pos = -1;
            if (!token.empty())
            {
                char *endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0')
                    pos = static_cast<int>(v);
            }
            if (pos < 0 || pos >= 32)
                continue;

            std::string key = "player_" + std::to_string(pos);
            std::string playerInfo = Mod::get()->getSavedValue<std::string>(key, "");
            if (playerInfo.empty())
                continue;

            // Expected: icon,color1,color2[,glow]
            std::stringstream playerSS(playerInfo);
            std::string infoToken;
            std::vector<int> values;
            while (std::getline(playerSS, infoToken, ','))
            {
                if (!infoToken.empty())
                {
                    char *endp = nullptr;
                    long v = std::strtol(infoToken.c_str(), &endp, 10);
                    if (endp && *endp == '\0')
                        values.push_back(static_cast<int>(v));
                }
            }
            if (values.size() < 3)
                continue;

            int iconId = values[0];
            int color1Idx = values[1];
            int color2Idx = values[2];

            auto playerNode = SimplePlayer::create(1);
            playerNode->updatePlayerFrame(iconId, static_cast<IconType>(0));
            // Store iconId on the node so popups can mirror it reliably
            playerNode->setTag(iconId);

            auto gm = GameManager::sharedState();
            playerNode->setColors(gm->colorForIdx(color1Idx), gm->colorForIdx(color2Idx));

            int col = pos % 8;
            int row = pos / 8; // 0..3
            int rowOffsets[4] = {75, 25, -25, -75};
            int playerX = (windowSize.width / 2) + xPositions[col];
            int playerY = (windowSize.height / 2) + rowOffsets[(row < 4 ? row : 3)];

            int perMaturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
            float playerScale = std::min(0.5f + (perMaturity * 0.06f), 0.8f);

            auto playerItem = CCMenuItemSpriteExtra::create(
                playerNode,
                this,
                menu_selector(ZenGardenLayer::onSimplePlayerClicked));
            playerItem->setID("purchased-player-" + std::to_string(pos));
            playerItem->setTag(pos);
            playerItem->setPosition(ccp(playerX, playerY));
            playerNode->setScale(playerScale);
            playerItem->setContentSize({40.f, 40.f});
            playerNode->setAnchorPoint(ccp(0.5f, 0.5f));
            playerNode->setPosition({playerItem->getContentSize().width / 2, playerItem->getContentSize().height / 2});
            playersMenu->addChild(playerItem);

            if (!firstPlayerAssigned)
            {
                ZenGardenLayer::m_simplePlayer = playerNode;
                m_activePos = pos;
                m_maturityLevel = perMaturity;
                m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
                m_lastOrbFeedTime = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
                firstPlayerAssigned = true;
            }
        }
    }
    else
    {
        m_activePos = -1;
        m_simplePlayer = nullptr;
    }

    // Recreate overlays for every player
    displayRequirementSprite();
    refreshSlotsMenuInteractable();
    // Keep selection flash in sync after reload
    if (m_moveFlashPos >= 0) startMoveSelectionFlash(m_moveFlashPos);
}

void ZenGardenLayer::updateSlotRequirementUI(int pos)
{
    // Overlays are shown for every slot now; refresh all
    displayRequirementSprite();
}

void ZenGardenLayer::onToggleMoveMode(CCObject* sender)
{
    // Toggle mode and update label visibility
    m_moveMode = !m_moveMode;
    m_moveSourcePos = -1;
    // Clear any selection flash when toggling off
    if (!m_moveMode)
    {
        stopMoveSelectionFlash();
    }
    if (m_moveModeLabel)
    {
        m_moveModeLabel->setVisible(m_moveMode);
        if (!m_moveMode)
            m_moveModeLabel->setString("Move Mode", true);
    }

    // Let user know
    //Notification::create(m_moveMode ? "Move Mode enabled" : "Move Mode disabled", NotificationIcon::Info, 1.f)->show();
    refreshSlotsMenuInteractable();
}

bool ZenGardenLayer::isSlotOccupied(int pos)
{
    // Check positions list quickly
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    if (positionsStr.empty()) return false;
    std::stringstream ss(positionsStr);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        if (token == std::to_string(pos)) return true;
    }
    return false;
}

void ZenGardenLayer::movePlayerTo(int fromPos, int toPos)
{
    if (fromPos == toPos || fromPos < 0 || toPos < 0 || fromPos >= 32 || toPos >= 32)
        return;

    // Ensure destination empty
    if (isSlotOccupied(toPos))
    {
        Notification::create("Destination slot is not empty", NotificationIcon::Error, 1.f)->show();
        return;
    }

    // Swap position in the positions list: replace fromPos with toPos
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    std::string out;
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;
        bool first = true;
        while (std::getline(ss, token, ','))
        {
            int p = -1;
            if (!token.empty())
            {
                char* endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0') p = static_cast<int>(v);
            }
            if (p < 0) continue;
            if (p == fromPos) p = toPos; // move
            if (!first) out += ",";
            out += std::to_string(p);
            first = false;
        }
    }
    Mod::get()->setSavedValue<std::string>("player_positions", out);

    // Move all per-slot data keys from fromPos to toPos
    auto moveStr = [&](const std::string& key){
        std::string val = Mod::get()->getSavedValue<std::string>(key + std::to_string(fromPos), "");
        Mod::get()->setSavedValue<std::string>(key + std::to_string(toPos), val);
        Mod::get()->setSavedValue<std::string>(key + std::to_string(fromPos), "");
    };
    auto moveInt = [&](const std::string& key){
        int val = Mod::get()->getSavedValue<int>(key + std::to_string(fromPos), 0);
        Mod::get()->setSavedValue<int>(key + std::to_string(toPos), val);
        Mod::get()->setSavedValue<int>(key + std::to_string(fromPos), 0);
    };
    auto moveI64 = [&](const std::string& key){
        int64_t val = Mod::get()->getSavedValue<int64_t>(key + std::to_string(fromPos), 0);
        Mod::get()->setSavedValue<int64_t>(key + std::to_string(toPos), val);
        Mod::get()->setSavedValue<int64_t>(key + std::to_string(fromPos), 0);
    };

    moveStr("player_");
    moveStr("player_name_");
    moveInt("player_maturity_");
    moveInt("player_orbs_fed_");
    moveI64("player_last_orb_feed_");
    moveInt("player_stars_fed_");
    moveInt("player_moons_fed_");
    moveInt("player_diamonds_fed_");
    moveI64("player_last_star_feed_");
    moveI64("player_last_moon_feed_");
    moveI64("player_last_diamond_feed_");

    // Update UI: reload and overlays
    reloadGardenFromSaves();
    displayRequirementSprite();
    refreshSlotsMenuInteractable();

    // Reset move state
    m_moveSourcePos = -1;
    stopMoveSelectionFlash();
    if (m_moveMode && m_moveModeLabel)
        m_moveModeLabel->setString("Move Mode", true);
    //Notification::create("Player moved", NotificationIcon::Success, 1.f)->show();
}

void ZenGardenLayer::onSlotClicked(CCObject* sender)
{
    if (!m_moveMode)
        return;
    int dst = -1;
    if (auto mi = typeinfo_cast<CCMenuItem*>(sender))
        dst = mi->getTag();
    if (dst < 0) return;

    if (m_moveSourcePos < 0)
    {
        //Notification::create("Select a player first", NotificationIcon::Warning, 1.f)->show();
        return;
    }
    if (isSlotOccupied(dst))
    {
        //Notification::create("That slot is already occupied", NotificationIcon::Error, 1.f)->show();
        return;
    }
    movePlayerTo(m_moveSourcePos, dst);
}

void ZenGardenLayer::refreshSlotsMenuInteractable()
{
    auto slotsMenu = this->getChildByID("slots-menu");
    if (!slotsMenu) return;
    if (auto children = slotsMenu->getChildren())
    {
        for (unsigned int i = 0; i < children->count(); ++i)
        {
            auto node = static_cast<CCNode*>(children->objectAtIndex(i));
            int pos = -1;
            if (auto mi = typeinfo_cast<CCMenuItem*>(node)) pos = mi->getTag();
            if (pos < 0) continue;
            bool enable = m_moveMode && !isSlotOccupied(pos);
            node->setVisible(enable); // show invisible hitboxes only when useful
        }
    }
}

void ZenGardenLayer::sellPlayerAtPos(int pos)
{
    if (pos < 0 || pos >= 32)
        return;

    // If the sold player is currently flashing selection, stop flashing and reset label
    if (m_moveFlashPos == pos) {
        stopMoveSelectionFlash();
        if (m_moveMode && m_moveModeLabel)
            m_moveModeLabel->setString("Move Mode", true);
    }

    // Compute payout
    int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    int payout = 750 + (200 * maturity);

    // Capture the player's name BEFORE clearing any saved data
    std::string soldPlayerName = Mod::get()->getSavedValue<std::string>(
        "player_name_" + std::to_string(pos), "");

    // Add to shards (money)
    ZenGardenLayer::m_diamondShards += payout;
    GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
    Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
    flashShards();

    // Remove from saves
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    std::string newPositions;
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;
        bool first = true;
        while (std::getline(ss, token, ','))
        {
            int p = -1;
            if (!token.empty())
            {
                char *endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0')
                    p = static_cast<int>(v);
            }
            if (p == pos)
                continue; // skip the sold one
            if (p >= 0)
            {
                if (!first)
                    newPositions += ",";
                newPositions += std::to_string(p);
                first = false;
            }
        }
    }
    Mod::get()->setSavedValue<std::string>("player_positions", newPositions);

    // Clear per-slot data
    Mod::get()->setSavedValue<std::string>("player_" + std::to_string(pos), "");
    Mod::get()->setSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_star_feed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(pos), 0);
    Mod::get()->setSavedValue<std::string>("player_name_" + std::to_string(pos), "");

    // Remove from UI
    if (auto playersMenu = this->getChildByID("players-menu"))
    {
        if (auto children = playersMenu->getChildren())
        {
            for (unsigned int i = 0; i < children->count(); ++i)
            {
                auto node = static_cast<CCNode *>(children->objectAtIndex(i));
                int tag = -1;
                if (auto mi = typeinfo_cast<CCMenuItem *>(node))
                    tag = mi->getTag();
                if (tag == pos)
                {
                    playersMenu->removeChild(node);
                    break;
                }
            }
        }
    }

    // Update active selection if needed
    if (m_activePos == pos)
    {
        m_activePos = -1;
        m_simplePlayer = nullptr;
    }

    auto fmod = FMODAudioEngine::sharedEngine();
    // @geode-ignore(unknown-resource)
    fmod->playEffect("chest08.ogg");

    // Notify
    if (soldPlayerName.empty())
        soldPlayerName = "Player";
    Notification::create("Sold " + soldPlayerName + " for " + std::to_string(payout), NotificationIcon::Success, 1.f)->show();

    // Refresh overlays
    displayRequirementSprite();
}

// Highlight the player at pos with a flashing yellow tint
void ZenGardenLayer::startMoveSelectionFlash(int pos)
{
    // Stop existing
    stopMoveSelectionFlash();
    if (pos < 0 || pos >= 32) return;
    auto sp = getPlayerNodeAtPos(pos);
    if (!sp) return;
    m_moveFlashPos = pos;
    // Build a repeating color-swap sequence using SimplePlayer::setColors for both primary and secondary
    auto delay = CCDelayTime::create(0.25f);
    auto toYellow = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::applyYellowColor));
    auto toOriginal = CCCallFuncN::create(this, callfuncN_selector(ZenGardenLayer::applyOriginalColors));
    auto seq = CCSequence::create(toYellow, delay, toOriginal, delay, nullptr);
    auto repeat = CCRepeatForever::create(seq);
    sp->runAction(repeat);
}

// Remove flashing highlight from whichever player is currently selected
void ZenGardenLayer::stopMoveSelectionFlash()
{
    if (m_moveFlashPos < 0) return;
    int flashedPos = m_moveFlashPos;
    auto sp = getPlayerNodeAtPos(flashedPos);
    m_moveFlashPos = -1;
    if (!sp) return;
    sp->stopAllActions();
    // Reset tint/opacity to defaults and restore original colors
    sp->setColor({255, 255, 255});
    sp->setOpacity(255);
    // Restore player colors by reapplying saved color indices
    // Fetch player info string to restore color indices
    std::string info = Mod::get()->getSavedValue<std::string>("player_" + std::to_string(flashedPos), "");
    // If we don't have reliable info for the flashing player, just set to white
    if (info.empty())
    {
        sp->setColor({255,255,255});
        return;
    }
    std::stringstream ss(info);
    std::string tok;
    std::vector<int> vals;
    while (std::getline(ss, tok, ','))
    {
        if (!tok.empty())
        {
            char* endp = nullptr;
            long v = std::strtol(tok.c_str(), &endp, 10);
            if (endp && *endp == '\0') vals.push_back(static_cast<int>(v));
        }
    }
    if (vals.size() >= 3)
    {
        auto gm = GameManager::sharedState();
        auto c1 = gm->colorForIdx(vals[1]);
        auto c2 = gm->colorForIdx(vals[2]);
        sp->setColors(c1, c2);
    }
}

// Callback: set both primary and secondary colors to yellow for flashing
void ZenGardenLayer::applyYellowColor(CCNode* node)
{
    auto sp = typeinfo_cast<SimplePlayer*>(node);
    if (!sp) return;
    ccColor3B yellow = {255, 255, 0};
    sp->setColors(yellow, yellow);
}

// Callback: restore original primary and secondary colors from saved indices
void ZenGardenLayer::applyOriginalColors(CCNode* node)
{
    auto sp = typeinfo_cast<SimplePlayer*>(node);
    if (!sp) return;
    int pos = -1;
    if (auto parentItem = typeinfo_cast<CCMenuItem*>(sp->getParent()))
        pos = parentItem->getTag();
    if (pos < 0) return;
    std::string info = Mod::get()->getSavedValue<std::string>("player_" + std::to_string(pos), "");
    if (info.empty())
    {
        sp->setColor({255,255,255});
        return;
    }
    std::stringstream ss(info);
    std::string tok;
    std::vector<int> vals;
    while (std::getline(ss, tok, ','))
    {
        if (!tok.empty())
        {
            char* endp = nullptr;
            long v = std::strtol(tok.c_str(), &endp, 10);
            if (endp && *endp == '\0') vals.push_back(static_cast<int>(v));
        }
    }
    if (vals.size() >= 3)
    {
        auto gm = GameManager::sharedState();
        auto c1 = gm->colorForIdx(vals[1]);
        auto c2 = gm->colorForIdx(vals[2]);
        sp->setColors(c1, c2);
    }
}

void ZenGardenLayer::showBronzeCoinReward(int pos)
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int posIdx = (pos >= 0 ? pos : (m_activePos >= 0 ? m_activePos : 0));
    int col = posIdx % 8;
    int row = posIdx / 8;
    int rowOffsets[4] = {75, 25, -25, -75};
    int baseY = rowOffsets[(row < 4 ? row : 3)];
    int coinX = (windowSize.width / 2) + xPositions[col];
    int coinY = (windowSize.height / 2) + baseY; // 30 pixels above the player

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
}

void ZenGardenLayer::showSilverCoinReward(int pos)
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int posIdx = (pos >= 0 ? pos : (m_activePos >= 0 ? m_activePos : 0));
    int col = posIdx % 8;
    int row = posIdx / 8;
    int rowOffsets[4] = {75, 25, -25, -75};
    int baseY = rowOffsets[(row < 4 ? row : 3)];
    int coinX = (windowSize.width / 2) + xPositions[col];
    int coinY = (windowSize.height / 2) + baseY; // 30 pixels above the player

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
}

void ZenGardenLayer::showGoldCoinReward(int pos)
{
    // Create a new coin sprite each time
    auto newCoinSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon2_001.png");
    if (!newCoinSprite)
        return;

    // Position the coin above the SimplePlayer
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    int xPositions[8] = {-190, -140, -90, -40, 40, 90, 140, 190}; // The x pos array (scary)
    int posIdx = (pos >= 0 ? pos : (m_activePos >= 0 ? m_activePos : 0));
    int col = posIdx % 8;
    int row = posIdx / 8;
    int rowOffsets[4] = {75, 25, -25, -75};
    int baseY = rowOffsets[(row < 4 ? row : 3)];
    int coinX = (windowSize.width / 2) + xPositions[col];
    int coinY = (windowSize.height / 2) + baseY; // 30 pixels above the player

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
}

void ZenGardenLayer::tryEmitCoins(int pos)
{
    // Generate random number between 1-100
    int randomChance = rand() % 100 + 1;

    // fmod engine
    auto fmod = FMODAudioEngine::sharedEngine();

    // 80% chance for bronze coin
    if (randomChance <= 80)
    {
        // @geode-ignore(unknown-resource)
        fmod->playEffect("gold01.ogg");
        showBronzeCoinReward(pos);
        ZenGardenLayer::m_diamondShards += 10;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
    }

    // 9% chance for silver coin (81-97)
    if (randomChance >= 81 && randomChance <= 97)
    {
        // @geode-ignore(unknown-resource)
        fmod->playEffect("gold01.ogg");
        showSilverCoinReward(pos);
        ZenGardenLayer::m_diamondShards += 100;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
        flashShards();
    }
    // 3% chance for gold coin (98-100)
    else if (randomChance >= 98 && randomChance <= 100)
    {
        // @geode-ignore(unknown-resource)
        fmod->playEffect("gold02.ogg");
        showGoldCoinReward(pos);
        ZenGardenLayer::m_diamondShards += 500;
        GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
        flashShards();
    }
}

void ZenGardenLayer::flashShards()
{
    if (!ZenGardenLayer::m_diamondShardsLabel)
        return;

    ZenGardenLayer::m_diamondShardsLabel->stopAllActions();
    ZenGardenLayer::m_diamondShardsLabel->setColor({0, 225, 255});
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
    // Build overlays for every player slot present in players-menu
    auto playersMenu = this->getChildByID("players-menu");
    if (!playersMenu)
        return;

    auto children = playersMenu->getChildren();
    if (!children)
        return;

    for (unsigned int i = 0; i < children->count(); ++i)
    {
        auto item = static_cast<CCNode *>(children->objectAtIndex(i));
        int pos = -1;
        if (auto mi = typeinfo_cast<CCMenuItem *>(item))
            pos = mi->getTag();
        if (pos < 0 || pos >= 32)
            continue;

        // Remove existing per-item overlays
        if (auto oldIcon = item->getChildByID("req-icon"))
            item->removeChild(oldIcon);
        if (auto oldLabel = item->getChildByID("req-label"))
            item->removeChild(oldLabel);

        int perMaturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
        CCSprite *icon = nullptr;

        if (perMaturity < 1)
        {
            bool canFeed = canFeedOrbForPos(pos);
            if (canFeed)
                icon = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
        }
        else if (perMaturity < 2)
        {
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_star_feed_" + std::to_string(pos), 0);
            int required = 17 * (1 << 1);
            if (currentTime - last >= required)
                icon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
        }
        else if (perMaturity < 3)
        {
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
            int required = 17 * (1 << 2);
            if (currentTime - last >= required)
                icon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
        }
        else
        {
            auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count();
            if (perMaturity >= 5)
            {
                int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
                int required = 17 * (1 << 5);
                if (currentTime - last >= required)
                    icon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
            }
            else
            {
                int levelForCooldown = perMaturity; // 3 or 4
                int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(pos), 0);
                int required = 17 * (1 << levelForCooldown);
                if (currentTime - last >= required)
                    icon = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
            }
        }

        if (icon)
        {
            icon->setID("req-icon");
            icon->setScale(0.4f);
            icon->setPosition({8.f, 32.f});
            item->addChild(icon, 500);

            // Show X/5 label for levels that require progress to next level
            if (perMaturity < 1)
            {
                int orbsFed = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
                auto lbl = CCLabelBMFont::create((std::to_string(orbsFed) + "/5").c_str(), "goldFont.fnt");
                lbl->setID("req-label");
                lbl->setScale(0.3f);
                lbl->setPosition({20.f, 10.f});
                item->addChild(lbl, 500);
            }
            else if (perMaturity < 2)
            {
                int starsFed = Mod::get()->getSavedValue<int>("player_stars_fed_" + std::to_string(pos), 0);
                auto lbl = CCLabelBMFont::create((std::to_string(starsFed) + "/5").c_str(), "goldFont.fnt");
                lbl->setID("req-label");
                lbl->setScale(0.3f);
                lbl->setPosition({20.f, 10.f});
                item->addChild(lbl, 500);
            }
            else if (perMaturity < 3)
            {
                int moonsFed = Mod::get()->getSavedValue<int>("player_moons_fed_" + std::to_string(pos), 0);
                auto lbl = CCLabelBMFont::create((std::to_string(moonsFed) + "/5").c_str(), "goldFont.fnt");
                lbl->setID("req-label");
                lbl->setScale(0.3f);
                lbl->setPosition({20.f, 10.f});
                item->addChild(lbl, 500);
            }
            else if (perMaturity < 5)
            {
                int diamondsFed = Mod::get()->getSavedValue<int>("player_diamonds_fed_" + std::to_string(pos), 0);
                auto lbl = CCLabelBMFont::create((std::to_string(diamondsFed) + "/5").c_str(), "goldFont.fnt");
                lbl->setID("req-label");
                lbl->setScale(0.3f);
                lbl->setPosition({20.f, 10.f});
                item->addChild(lbl, 500);
            }
            // maturity >= 5: requires moons but no further leveling, so no progress label
        }
    }
}

void ZenGardenLayer::hideRequirementSprite()
{
    // Remove all per-item overlays
    if (auto playersMenu = this->getChildByID("players-menu"))
    {
        if (auto children = playersMenu->getChildren())
        {
            for (unsigned int i = 0; i < children->count(); ++i)
            {
                auto item = static_cast<CCNode *>(children->objectAtIndex(i));
                if (auto ic = item->getChildByID("req-icon"))
                    item->removeChild(ic);
                // if (auto lb = item->getChildByID("req-label")) item->removeChild(lb);
            }
        }
    }

    // Clear legacy single refs if any
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

        // Reset counters for the requirement that was just completed
        if (m_maturityLevel == 1)
        {
            // 0 -> 1 was ORBs
            m_orbsFeeded = 0;
            if (m_activePos >= 0)
                Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(m_activePos), 0);
        }
        else if (m_maturityLevel == 2)
        {
            // 1 -> 2 was STARs
            if (m_activePos >= 0)
                Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(m_activePos), 0);
            else
                Mod::get()->setSavedValue<int>("player_stars_fed", 0);
        }
        else if (m_maturityLevel == 3)
        {
            // 2 -> 3 was MOONs
            if (m_activePos >= 0)
                Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(m_activePos), 0);
            else
                Mod::get()->setSavedValue<int>("player_moons_fed", 0);
        }
        else if (m_maturityLevel == 4 || m_maturityLevel == 5)
        {
            // 3 -> 4 and 4 -> 5 are DIAMONDs
            if (m_activePos >= 0)
                Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(m_activePos), 0);
            else
                Mod::get()->setSavedValue<int>("player_diamonds_fed", 0);
        }

        // Save updated maturity level (per active player if available)
        if (m_activePos >= 0)
            Mod::get()->setSavedValue<int>("player_maturity_" + std::to_string(m_activePos), m_maturityLevel);
        else
            Mod::get()->setSavedValue<int>("player_maturity", m_maturityLevel);

        // Update visuals
        updatePlayerMaturityVisuals();

        // Show message
        std::string playerName = Mod::get()->getSavedValue<std::string>(
            "player_name_" + std::to_string(m_activePos), "");
        if (playerName.empty())
            playerName = "Player";
        std::string message = playerName + " grew to maturity level " + std::to_string(m_maturityLevel) + "!";
        Notification::create(message, NotificationIcon::Info, 1.f)->show();
    }
}

bool ZenGardenLayer::canFeedOrb()
{
    // system time
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    int64_t timeSinceLastFeed = currentTime - m_lastOrbFeedTime;

    // log::debug("Time since last feed: {} seconds", timeSinceLastFeed);

    // Cooldown doubles each level; for orbs, use current maturity level index
    int required = 17 * (1 << std::max(0, m_maturityLevel));
    return timeSinceLastFeed >= required;
}

void ZenGardenLayer::displayOrbCooldownMessage()
{
    // accurate seconds calculation
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    int64_t timeSinceLastFeed = currentTime - m_lastOrbFeedTime;
    int required = 17 * (1 << std::max(0, m_maturityLevel));
    int64_t remainingTime = required - timeSinceLastFeed;

    // don't show negative time
    int secondsRemaining = static_cast<int>(remainingTime > 0 ? remainingTime : 0);

    std::string message = "Please wait " + std::to_string(secondsRemaining) + " seconds.";
    // Prefix with active player's name if available
    std::string playerName;
    if (m_activePos >= 0)
        playerName = Mod::get()->getSavedValue<std::string>("player_name_" + std::to_string(m_activePos), "");
    if (playerName.empty())
        playerName = "Player";

    log::debug("Feeding Cooldown: {}", message);
    Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
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
    if (m_diamondCurrencyIcon && m_diamondShardsLabel)
    {
        float labelWidth = m_diamondShardsLabel->getContentWidth() * m_diamondShardsLabel->getScale();
        m_diamondCurrencyIcon->setPosition(ccp(m_diamondShardsLabel->getPositionX() - (labelWidth / 2) - 15, m_diamondShardsLabel->getPositionY()));
    }

    // Update per-slot overlays: toggle icon by cooldown and refresh X/5 for current requirement
    if (auto playersMenu = this->getChildByID("players-menu"))
    {
        if (auto children = playersMenu->getChildren())
        {
            for (unsigned int i = 0; i < children->count(); ++i)
            {
                auto item = static_cast<CCNode *>(children->objectAtIndex(i));
                int pos = -1;
                if (auto mi = typeinfo_cast<CCMenuItem *>(item))
                    pos = mi->getTag();
                if (pos < 0 || pos >= 32)
                    continue;
                int perMaturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
                auto icon = item->getChildByID("req-icon");
                auto lblNode = item->getChildByID("req-label");
                if (perMaturity == 0)
                {
                    bool canFeed = canFeedOrbForPos(pos);
                    if (!canFeed)
                    {
                        if (icon)
                            item->removeChild(icon);
                    }
                    else if (!icon)
                    {
                        auto newIcon = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
                        if (newIcon)
                        {
                            newIcon->setID("req-icon");
                            newIcon->setScale(0.4f);
                            newIcon->setPosition({8.f, 32.f});
                            item->addChild(newIcon, 500);
                        }
                    }
                    auto lbl = typeinfo_cast<CCLabelBMFont *>(lblNode);
                    if (!lbl)
                    {
                        lbl = CCLabelBMFont::create("0/5", "goldFont.fnt");
                        lbl->setID("req-label");
                        lbl->setScale(0.3f);
                        lbl->setPosition({20.f, 10.f});
                        item->addChild(lbl, 500);
                    }
                    int orbsFed = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
                    lbl->setString((std::to_string(orbsFed) + "/5").c_str(), true);
                }
                else if (perMaturity == 1)
                {
                    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count();
                    int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_star_feed_" + std::to_string(pos), 0);
                    int required = 17 * (1 << 1);
                    if (currentTime - last < required)
                    {
                        if (icon)
                            item->removeChild(icon);
                    }
                    else if (!icon)
                    {
                        auto newIcon = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
                        if (newIcon)
                        {
                            newIcon->setID("req-icon");
                            newIcon->setScale(0.4f);
                            newIcon->setPosition({8.f, 32.f});
                            item->addChild(newIcon, 500);
                        }
                    }
                    auto lbl = typeinfo_cast<CCLabelBMFont *>(lblNode);
                    if (!lbl)
                    {
                        lbl = CCLabelBMFont::create("0/5", "goldFont.fnt");
                        lbl->setID("req-label");
                        lbl->setScale(0.3f);
                        lbl->setPosition({20.f, 10.f});
                        item->addChild(lbl, 500);
                    }
                    int starsFed = Mod::get()->getSavedValue<int>("player_stars_fed_" + std::to_string(pos), 0);
                    lbl->setString((std::to_string(starsFed) + "/5").c_str(), true);
                }
                else if (perMaturity == 2)
                {
                    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count();
                    int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
                    int required = 17 * (1 << 2);
                    if (currentTime - last < required)
                    {
                        if (icon)
                            item->removeChild(icon);
                    }
                    else if (!icon)
                    {
                        auto newIcon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
                        if (newIcon)
                        {
                            newIcon->setID("req-icon");
                            newIcon->setScale(0.4f);
                            newIcon->setPosition({8.f, 32.f});
                            item->addChild(newIcon, 500);
                        }
                    }
                    auto lbl = typeinfo_cast<CCLabelBMFont *>(lblNode);
                    if (!lbl)
                    {
                        lbl = CCLabelBMFont::create("0/5", "goldFont.fnt");
                        lbl->setID("req-label");
                        lbl->setScale(0.3f);
                        lbl->setPosition({20.f, 10.f});
                        item->addChild(lbl, 500);
                    }
                    int moonsFed = Mod::get()->getSavedValue<int>("player_moons_fed_" + std::to_string(pos), 0);
                    lbl->setString((std::to_string(moonsFed) + "/5").c_str(), true);
                }
                else if (perMaturity >= 3 && perMaturity < 5)
                {
                    int levelForCooldown = perMaturity;
                    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count();
                    int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(pos), 0);
                    int required = 17 * (1 << levelForCooldown);
                    if (currentTime - last < required)
                    {
                        if (icon)
                            item->removeChild(icon);
                    }
                    else if (!icon)
                    {
                        auto newIcon = CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
                        if (newIcon)
                        {
                            newIcon->setID("req-icon");
                            newIcon->setScale(0.4f);
                            newIcon->setPosition({8.f, 32.f});
                            item->addChild(newIcon, 500);
                        }
                    }
                    auto lbl = typeinfo_cast<CCLabelBMFont *>(lblNode);
                    if (!lbl)
                    {
                        lbl = CCLabelBMFont::create("0/5", "goldFont.fnt");
                        lbl->setID("req-label");
                        lbl->setScale(0.3f);
                        lbl->setPosition({20.f, 10.f});
                        item->addChild(lbl, 500);
                    }
                    int diamondsFed = Mod::get()->getSavedValue<int>("player_diamonds_fed_" + std::to_string(pos), 0);
                    lbl->setString((std::to_string(diamondsFed) + "/5").c_str(), true);
                }
                else if (perMaturity >= 5)
                {
                    // At 5+, requires moons but no further leveling; show moon icon only when off cooldown, no label
                    int levelForCooldown = 5;
                    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count();
                    int64_t last = Mod::get()->getSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
                    int required = 17 * (1 << levelForCooldown);
                    if (currentTime - last < required)
                    {
                        if (icon)
                            item->removeChild(icon);
                    }
                    else if (!icon)
                    {
                        auto newIcon = CCSprite::createWithSpriteFrameName("GJ_moonsIcon_001.png");
                        if (newIcon)
                        {
                            newIcon->setID("req-icon");
                            newIcon->setScale(0.4f);
                            newIcon->setPosition({8.f, 32.f});
                            item->addChild(newIcon, 500);
                        }
                    }
                    if (lblNode)
                        item->removeChild(lblNode);
                }
            }
        }
    }
}

void ZenGardenLayer::onResetProgress(CCObject *sender)
{
    if (m_diamondShards >= 1000)
    {
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
    }
    else
    {
        Notification::create("You don't have enough to reset! Required 1000 Diamond Shards", NotificationIcon::Error, 1.f)->show();
    }
}

void ZenGardenLayer::confirmResetProgress()
{
    // Reset all saved values to default
    Mod::get()->setSavedValue<int>("player_maturity", 0);
    Mod::get()->setSavedValue<int>("player_orbs_fed", 0);
    Mod::get()->setSavedValue<float>("player_last_orb_feed", 0.0f);
    Mod::get()->setSavedValue<int>("stars", 0);
    Mod::get()->setSavedValue<int>("moons", 0);
    Mod::get()->setSavedValue<int>("diamonds", 0);
    Mod::get()->setSavedValue<int>("money", 0);

    // Clear purchased players and per-slot data
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;
        while (std::getline(ss, token, ','))
        {
            int pos = -1;
            if (!token.empty())
            {
                char *endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0')
                    pos = static_cast<int>(v);
            }
            if (pos >= 0 && pos < 32)
            {
                Mod::get()->setSavedValue<std::string>("player_" + std::to_string(pos), "");
                Mod::get()->setSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int64_t>("player_last_star_feed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
                Mod::get()->setSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(pos), 0);
            }
        }
    }
    Mod::get()->setSavedValue<std::string>("player_positions", "");
    Mod::get()->setSavedValue<bool>("shop_item_0_purchased", false);
    Mod::get()->setSavedValue<bool>("shop_item_1_purchased", false);

    // Update in-memory values
    m_maturityLevel = 0;
    m_orbsFeeded = 0;
    m_lastOrbFeedTime = 0;
    m_starCount = 0;
    m_moonCount = 0;
    m_diamondCount = 0;
    m_diamondShards = GameStatsManager::sharedState()->getStat("29");
    m_activePos = -1;

    // Immediately clear UI: remove all players and requirement overlays
    if (auto playersMenu = this->getChildByID("players-menu"))
    {
        this->removeChild(playersMenu);
    }
    hideRequirementSprite();
    // Rebuild to ensure empty state is shown
    reloadGardenFromSaves();

    // Show confirmation message
    FLAlertLayer::create(
        "Progress Reset",
        "Your progress has been reset to default values!",
        "OK")
        ->show();
}

// I accidentally wipped my diamond shards so this is needed for me xD
// also using this is considered cheating
void ZenGardenLayer::cheat()
{

    ZenGardenLayer::m_diamondShards += 1000;
    GameStatsManager::sharedState()->setStat("29", ZenGardenLayer::m_diamondShards);
    Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);
    flashShards();
}

// Clears all per-slot feed cooldown timestamps so all players can be fed immediately
void ZenGardenLayer::clearAllFeedCooldowns()
{
    // Gather occupied positions and zero out their last-feed timers
    auto positions = ZenGardenLayer::getOccupiedPositions();
    if (positions)
    {
        for (unsigned int i = 0; i < positions->count(); ++i)
        {
            int pos = static_cast<CCInteger *>(positions->objectAtIndex(i))->getValue();
            Mod::get()->setSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
            Mod::get()->setSavedValue<int64_t>("player_last_star_feed_" + std::to_string(pos), 0);
            Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(pos), 0);
            Mod::get()->setSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(pos), 0);
        }
    }

    // Refresh requirement overlays to reflect no cooldown
    displayRequirementSprite();
}

// Get an array of positions that currently have SimplePlayer icons
CCArray *ZenGardenLayer::getOccupiedPositions()
{
    auto positions = CCArray::create();

    // Get saved player positions
    std::string positionsStr = Mod::get()->getSavedValue<std::string>("player_positions", "");
    if (!positionsStr.empty())
    {
        std::stringstream ss(positionsStr);
        std::string token;

        while (std::getline(ss, token, ','))
        {
            int pos = -1;
            if (!token.empty())
            {
                char *endp = nullptr;
                long v = std::strtol(token.c_str(), &endp, 10);
                if (endp && *endp == '\0')
                    pos = static_cast<int>(v);
            }
            if (pos >= 0)
                positions->addObject(CCInteger::create(pos));
        }
    }

    return positions;
}

// Add a random SimplePlayer to the garden at the first free position
bool ZenGardenLayer::addRandomSimplePlayer()
{
    // Get occupied positions
    auto occupiedPositions = getOccupiedPositions();

    // Find the first free position (0-31)
    int freePosition = -1;
    for (int i = 0; i < 32; i++)
    {
        bool isOccupied = false;
        for (int j = 0; j < occupiedPositions->count(); j++)
        {
            auto pos = static_cast<CCInteger *>(occupiedPositions->objectAtIndex(j))->getValue();
            if (pos == i)
            {
                isOccupied = true;
                break;
            }
        }

        if (!isOccupied)
        {
            freePosition = i;
            break;
        }
    }

    // No free positions
    if (freePosition == -1)
    {
        return false;
    }

    // Generate random icon (1-42 typical for cube) and color indices (Geometry Dash palette indices)
    int randomIcon = rand() % 42 + 1;
    int randomColor1 = rand() % 42 + 1;
    int randomColor2 = rand() % 42 + 1;
    int randomGlow = (rand() % 100) < 25 ? 1 : 0; // 25% chance to have glow

    // Save the new player info: icon,color1,color2,glow
    std::string key = "player_" + std::to_string(freePosition);
    std::string playerInfo = std::to_string(randomIcon) + "," +
                             std::to_string(randomColor1) + "," +
                             std::to_string(randomColor2) + "," +
                             std::to_string(randomGlow);
    Mod::get()->setSavedValue<std::string>(key, playerInfo);

    // Initialize per-slot stats for the new player
    Mod::get()->setSavedValue<int>("player_maturity_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_star_feed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_moon_feed_" + std::to_string(freePosition), 0);
    Mod::get()->setSavedValue<int64_t>("player_last_diamond_feed_" + std::to_string(freePosition), 0);

    // Save the new position to the list of occupied positions
    occupiedPositions->addObject(CCInteger::create(freePosition));
    std::string positionsStr = "";
    for (int i = 0; i < occupiedPositions->count(); i++)
    {
        auto pos = static_cast<CCInteger *>(occupiedPositions->objectAtIndex(i))->getValue();
        positionsStr += std::to_string(pos);
        if (i < occupiedPositions->count() - 1)
        {
            positionsStr += ",";
        }
    }
    Mod::get()->setSavedValue<std::string>("player_positions", positionsStr);

    return true;
}

void ZenGardenLayer::keyDown(enumKeyCodes key)
{
    bool isCheat = Mod::get()->getSettingValue<bool>("cheat");

    // Check for the "J" key press (74 is the key code for 'J')
    if (key == KEY_J && isCheat)
    {
        cheat();
    }

    // clear all feed cooldowns when cheats are enabled
    if (key == KEY_K && isCheat)
    {
        clearAllFeedCooldowns();
        Notification::create("All feed cooldowns cleared.", NotificationIcon::Success, 1.f)->show();
        return;
    }

    CCLayer::keyDown(key);
}

void ZenGardenLayer::keyBackClicked()
{
    onBack(nullptr);
}

void ZenGardenLayer::onBack(CCObject *sender)
{
    CCDirector::sharedDirector()->popScene();
    GameManager::sharedState()->fadeInMenuMusic();
}

void ZenGardenLayer::onShop(CCObject *sender)
{
    Mod::get()->setSavedValue<int>("stars", ZenGardenLayer::m_starCount);
    Mod::get()->setSavedValue<int>("moons", ZenGardenLayer::m_moonCount);
    Mod::get()->setSavedValue<int>("diamonds", ZenGardenLayer::m_diamondCount);
    Mod::get()->setSavedValue<int>("money", ZenGardenLayer::m_diamondShards);

    auto scene = CCScene::create();
    scene->addChild(ZenGardenShopLayer::create());

    CCDirector::get()->pushScene(CCTransitionMoveInT::create(.5f, scene));
}

void ZenGardenLayer::onFeedOrbs(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 1)
    {
        ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(100, 100, 100));
        ZenGardenLayer::m_feedStarsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(255, 255, 255));

        ZenGardenLayer::m_selectedItem = 1;
        if (ZenGardenLayer::m_orbsHoverSprite)
            ZenGardenLayer::m_orbsHoverSprite->setVisible(true);
        if (ZenGardenLayer::m_starsHoverSprite)
            ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_moonsHoverSprite)
            ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_diamondsHoverSprite)
            ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedStars(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 2)
    {
        ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedStarsButton->setColor(ccc3(100, 100, 100));
        ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(255, 255, 255));

        ZenGardenLayer::m_selectedItem = 2;
        if (ZenGardenLayer::m_orbsHoverSprite)
            ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_starsHoverSprite)
            ZenGardenLayer::m_starsHoverSprite->setVisible(true);
        if (ZenGardenLayer::m_moonsHoverSprite)
            ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_diamondsHoverSprite)
            ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_feedStarsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_starsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedMoons(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 3)
    {
        ZenGardenLayer::m_selectedItem = 3;

        ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedStarsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(100, 100, 100));
        ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(255, 255, 255));

        if (ZenGardenLayer::m_orbsHoverSprite)
            ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_starsHoverSprite)
            ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_moonsHoverSprite)
            ZenGardenLayer::m_moonsHoverSprite->setVisible(true);
        if (ZenGardenLayer::m_diamondsHoverSprite)
            ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
    }
}

void ZenGardenLayer::onFeedDiamonds(CCObject *sender)
{
    if (ZenGardenLayer::m_selectedItem != 4)
    {
        ZenGardenLayer::m_feedOrbsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedStarsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedMoonsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(100, 100, 100));

        ZenGardenLayer::m_selectedItem = 4;
        if (ZenGardenLayer::m_orbsHoverSprite)
            ZenGardenLayer::m_orbsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_starsHoverSprite)
            ZenGardenLayer::m_starsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_moonsHoverSprite)
            ZenGardenLayer::m_moonsHoverSprite->setVisible(false);
        if (ZenGardenLayer::m_diamondsHoverSprite)
            ZenGardenLayer::m_diamondsHoverSprite->setVisible(true);
    } else {
        ZenGardenLayer::m_selectedItem = 0;
        ZenGardenLayer::m_feedDiamondsButton->setColor(ccc3(255, 255, 255));
        ZenGardenLayer::m_diamondsHoverSprite->setVisible(false);
    }
}

// Helpers to work with specific slot indices
SimplePlayer *ZenGardenLayer::getPlayerNodeAtPos(int pos)
{
    if (auto playersMenu = this->getChildByID("players-menu"))
    {
        if (auto children = playersMenu->getChildren())
        {
            for (unsigned int i = 0; i < children->count(); ++i)
            {
                auto node = static_cast<CCNode *>(children->objectAtIndex(i));
                int tag = -1;
                if (auto mi = typeinfo_cast<CCMenuItem *>(node))
                    tag = mi->getTag();
                if (tag == pos)
                {
                    if (node->getChildren() && node->getChildren()->count() > 0)
                    {
                        return typeinfo_cast<SimplePlayer *>(node->getChildren()->objectAtIndex(0));
                    }
                }
            }
        }
    }
    return nullptr;
}

void ZenGardenLayer::updatePlayerMaturityVisualsForPos(int pos)
{
    int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    float playerScale = std::min(0.5f + (maturity * 0.06f), 0.8f);
    if (auto sp = getPlayerNodeAtPos(pos))
        sp->setScale(playerScale);
}

void ZenGardenLayer::handlePlayerGrowthForPos(int pos)
{
    int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    if (maturity < 5)
    {
        maturity++;
        // Reset counters corresponding to the requirement completed
        if (maturity == 1)
        {
            // 0 -> 1 was ORBs
            Mod::get()->setSavedValue<int>("player_orbs_fed_" + std::to_string(pos), 0);
        }
        else if (maturity == 2)
        {
            // 1 -> 2 was STARs
            Mod::get()->setSavedValue<int>("player_stars_fed_" + std::to_string(pos), 0);
        }
        else if (maturity == 3)
        {
            // 2 -> 3 was MOONs
            Mod::get()->setSavedValue<int>("player_moons_fed_" + std::to_string(pos), 0);
        }
        else if (maturity == 4 || maturity == 5)
        {
            // 3 -> 4 and 4 -> 5 are DIAMONDs
            Mod::get()->setSavedValue<int>("player_diamonds_fed_" + std::to_string(pos), 0);
        }
        Mod::get()->setSavedValue<int>("player_maturity_" + std::to_string(pos), maturity);
        updatePlayerMaturityVisualsForPos(pos);

        std::string playerName = Mod::get()->getSavedValue<std::string>(
            "player_name_" + std::to_string(m_activePos), "");
        if (playerName.empty())
            playerName = "Player";

        Notification::create(playerName + " grew to maturity level " + std::to_string(maturity) + "!", NotificationIcon::Info, 1.f)->show();
    }
}

bool ZenGardenLayer::canFeedOrbForPos(int pos)
{
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
    int64_t timeSinceLastFeed = currentTime - lastFeed;
    int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    int required = 17 * (1 << std::max(0, maturity));
    return timeSinceLastFeed >= required;
}

void ZenGardenLayer::displayOrbCooldownMessageForPos(int pos)
{
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    int64_t lastFeed = Mod::get()->getSavedValue<int64_t>("player_last_orb_feed_" + std::to_string(pos), 0);
    int64_t timeSinceLastFeed = currentTime - lastFeed;
    int maturity = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(pos), 0);
    int required = 17 * (1 << std::max(0, maturity));
    int64_t remainingTime = required - timeSinceLastFeed;
    int secondsRemaining = static_cast<int>(remainingTime > 0 ? remainingTime : 0);
    log::debug("Feeding Cooldown (pos {}): {} seconds remaining", pos, secondsRemaining);
    // Prefix with this player's name and include remaining seconds
    std::string playerName = Mod::get()->getSavedValue<std::string>(
        "player_name_" + std::to_string(pos), "");
    if (playerName.empty())
        playerName = "Player";
    Notification::create(playerName + " is on feeding cooldown", NotificationIcon::Loading, 1.f)->show();
}