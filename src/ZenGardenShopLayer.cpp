#include "ZenGardenShopLayer.hpp"
#include "ZenGardenLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/ui/Notification.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>

ZenGardenShopLayer *ZenGardenShopLayer::create()
{
    auto zenGardenShopLayer = new ZenGardenShopLayer();
    auto playerIcon1 = SimplePlayer::create(1);
    playerIcon1->updatePlayerFrame(1, static_cast<IconType>(1));

    if (zenGardenShopLayer && zenGardenShopLayer->init())
    {
        zenGardenShopLayer->autorelease();
        return zenGardenShopLayer;
    }

    CC_SAFE_DELETE(zenGardenShopLayer);
    return nullptr;
}

void ZenGardenShopLayer::onBack(CCObject *sender)
{
    CCDirector::sharedDirector()->popScene();
}

void ZenGardenShopLayer::onBuyStars(CCObject *sender)
{
    int cost = 100;
    int currentShards = Mod::get()->getSavedValue<int>("money", 0);

    if (currentShards < cost)
    {
        FLAlertLayer::create(
            "Insufficient Funds",
            "You need 100 diamond shards to purchase 5 <cy>stars</c>!",
            "OK")
            ->show();
        return;
    }

    geode::createQuickPopup(
        "Buy Stars",
        "Are you sure you want to buy 5 <cy>stars</c> for 100 diamond shards?",
        "Yes", "No",
        [this, cost](FLAlertLayer *, bool btn2)
        {
            if (!btn2)
            {
                // Get current values
                int currentShards = Mod::get()->getSavedValue<int>("money", 0);
                int currentStars = Mod::get()->getSavedValue<int>("stars", 0);

                // Update values
                currentShards -= cost;
                currentStars += 5;

                // Save the updated values
                Mod::get()->setSavedValue<int>("money", currentShards);
                Mod::get()->setSavedValue<int>("stars", currentStars);

                // Update diamond shards count in ZenGardenLayer
                ZenGardenLayer::m_diamondShards = currentShards;

                // update the static count in ZenGardenLayer if it exists
                ZenGardenLayer::m_starCount = currentStars;

                // Update the diamond shards label
                updateDiamondShardsLabel();

                // Update ZenGardenLayer labels if it exists
                updateZenGardenLayerLabels();

                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cy>stars</c>!",
                    "OK")
                    ->show();
            }
        });
}

void ZenGardenShopLayer::onBuyMoons(CCObject *sender)
{
    int cost = 200;
    int currentShards = Mod::get()->getSavedValue<int>("money", 0);

    if (currentShards < cost)
    {
        FLAlertLayer::create(
            "Insufficient Funds",
            "You need 200 diamond shards to purchase 5 <cb>moons</c>!",
            "OK")
            ->show();
        return;
    }

    geode::createQuickPopup(
        "Buy Moons",
        "Are you sure you want to buy 5 <cb>moons</c> for 200 diamond shards?",
        "Yes", "No",
        [this, cost](FLAlertLayer *, bool btn2)
        {
            if (!btn2)
            {
                // Get current values
                int currentShards = Mod::get()->getSavedValue<int>("money", 0);
                int currentMoons = Mod::get()->getSavedValue<int>("moons", 0);

                // Update values
                currentShards -= cost;
                currentMoons += 5;

                // Save the updated values
                Mod::get()->setSavedValue<int>("money", currentShards);
                Mod::get()->setSavedValue<int>("moons", currentMoons);

                // Update diamond shards count in ZenGardenLayer
                ZenGardenLayer::m_diamondShards = currentShards;

                // update the static count in ZenGardenLayer if it exists
                ZenGardenLayer::m_moonCount = currentMoons;

                // Update the diamond shards label
                updateDiamondShardsLabel();

                // Update ZenGardenLayer labels if it exists
                updateZenGardenLayerLabels();

                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cb>moons</c>!",
                    "OK")
                    ->show();
            }
        });
}

void ZenGardenShopLayer::onBuyDiamonds(CCObject *sender)
{
    int cost = 500;
    int currentShards = Mod::get()->getSavedValue<int>("money", 0);

    if (currentShards < cost)
    {
        FLAlertLayer::create(
            "Insufficient Funds",
            "You need 500 diamond shards to purchase 5 <cb>diamonds</c>!",
            "OK")
            ->show();
        return;
    }

    geode::createQuickPopup(
        "Buy Diamonds",
        "Are you sure you want to buy 5 <cb>diamonds</c> for 500 diamond shards?",
        "Yes", "No",
        [this, cost](FLAlertLayer *, bool btn2)
        {
            if (!btn2)
            {
                // Get current values
                int currentShards = Mod::get()->getSavedValue<int>("money", 0);
                int currentDiamonds = Mod::get()->getSavedValue<int>("diamonds", 0);

                // Update values
                currentShards -= cost;
                currentDiamonds += 5;

                // Save the updated values
                Mod::get()->setSavedValue<int>("money", currentShards);
                Mod::get()->setSavedValue<int>("diamonds", currentDiamonds);

                // Update diamond shards count in ZenGardenLayer
                ZenGardenLayer::m_diamondShards = currentShards;

                // update the static count in ZenGardenLayer if it exists
                ZenGardenLayer::m_diamondCount = currentDiamonds;

                // Update the diamond shards label
                updateDiamondShardsLabel();

                // Update ZenGardenLayer labels if it exists
                updateZenGardenLayerLabels();

                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased 5 <cb>diamonds</c>!",
                    "OK")
                    ->show();
            }
        });
}

bool ZenGardenShopLayer::init()
{
    if (!CCLayer::init())
    {
        return false;
    }

    // Update the icons label and diamond shards label to refresh every second
    this->schedule(schedule_selector(ZenGardenShopLayer::updateIconsLabel), 1.0f);
    
    // Load purchase state from saved values
    m_itemPurchased[0] = Mod::get()->getSavedValue<bool>("shop_item_1_purchased", false);
    m_itemPurchased[1] = Mod::get()->getSavedValue<bool>("shop_item_2_purchased", false);
    
    // Check for restock
    checkForRestock();

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

    // Add diamond shards display on the top left
    int diamondShards = Mod::get()->getSavedValue<int>("money", 0);
    m_diamondShardsLabel = CCLabelBMFont::create(std::to_string(diamondShards).c_str(), "bigFont.fnt");
    m_diamondShardsLabel->setScale(0.65f);
    m_diamondShardsLabel->setID("shop-money-count");
    m_diamondShardsLabel->setPosition(80, windowSize.height - 25);

    // Add diamond icon next to the label
    auto diamondIcon = CCSprite::createWithSpriteFrameName("currencyDiamondIcon_001.png");
    diamondIcon->setScale(0.8f);
    diamondIcon->setID("shop-diamond-currency-icon");
    diamondIcon->setPosition(ccp(m_diamondShardsLabel->getPositionX() - (m_diamondShardsLabel->getContentWidth() * m_diamondShardsLabel->getScale() / 2) - 15, m_diamondShardsLabel->getPositionY()));

    this->addChild(diamondIcon);
    this->addChild(m_diamondShardsLabel);

    // Get the local time
    std::time_t currentTime = std::time(nullptr);
    std::tm local_tm = *std::localtime(&currentTime);

    // Set the time to tomorrow at midnight
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    local_tm.tm_mday += 1;

    // Calculate the time until midnight
    std::time_t midnightTime = std::mktime(&local_tm);
    std::time_t remainingSeconds = midnightTime - currentTime;

    // Convert difference to hours, minutes, and seconds
    int hours = remainingSeconds / 3600;
    int minutes = (remainingSeconds % 3600) / 60;
    int seconds = remainingSeconds % 60;

    // Create the label
    ZenGardenShopLayer::m_iconsLabel = CCLabelBMFont::create(
        (std::string("Icons (restocks in ") + (hours < 10 ? "0" : "") + std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds) + ")").c_str(),
        "bigFont.fnt");
    ZenGardenShopLayer::m_iconsLabel->setScale(0.7f);
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
    
    // Create menus for SimplePlayer icons
    auto playerIconsMenu = CCMenu::create();
    playerIconsMenu->setPosition({0, 0});
    playerIconsMenu->setID("player-icons-menu");
    this->addChild(playerIconsMenu);
    
    // Create SimplePlayer1 as a clickable menu item
    auto playerIcon1 = SimplePlayer::create(1);
    playerIcon1->setColor({128, 128, 128}); // Grey color
    playerIcon1->setScale(1.0f);
    
    auto playerMenuItem1 = CCMenuItemSpriteExtra::create(
        playerIcon1,
        this,
        menu_selector(ZenGardenShopLayer::onBuySimplePlayer)
    );
    playerMenuItem1->setPosition(ccp(windowSize.width / 2 - 82, windowSize.height - 155));
    playerMenuItem1->setID("simple-player-icon-1");
    playerMenuItem1->setTag(0); // Tag 0 for first slot
    playerIconsMenu->addChild(playerMenuItem1);
    
    // Add question mark over the SimplePlayer
    auto questionMark1 = CCLabelBMFont::create("?", "bigFont.fnt");
    questionMark1->setPosition(ccp(windowSize.width / 2 - 82, windowSize.height - 155));
    questionMark1->setScale(1.0f);
    questionMark1->setID("question-mark-1");
    this->addChild(questionMark1);
    
    // Create SimplePlayer2 as a clickable menu item
    auto playerIcon2 = SimplePlayer::create(1);
    playerIcon2->setColor({128, 128, 128}); // Grey color
    playerIcon2->setScale(1.0f);
    
    auto playerMenuItem2 = CCMenuItemSpriteExtra::create(
        playerIcon2,
        this,
        menu_selector(ZenGardenShopLayer::onBuySimplePlayer)
    );
    playerMenuItem2->setPosition(ccp(windowSize.width / 2 + 82, windowSize.height - 155));
    playerMenuItem2->setID("simple-player-icon-2");
    playerMenuItem2->setTag(1); // Tag 1 for second slot
    playerIconsMenu->addChild(playerMenuItem2);
    
    // Add question mark over the SimplePlayer
    auto questionMark2 = CCLabelBMFont::create("?", "bigFont.fnt");
    questionMark2->setPosition(ccp(windowSize.width / 2 + 82, windowSize.height - 155));
    questionMark2->setScale(1.0f);
    questionMark2->setID("question-mark-2");
    this->addChild(questionMark2);

    auto statsLabel = CCLabelBMFont::create("Stars/Moons/Diamonds", "bigFont.fnt");
    statsLabel->setPosition(windowSize.width / 2, windowSize.height - 225);
    statsLabel->setScale(0.7f);
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
    
    // Add price label for SimplePlayer icons
    auto priceLabel = CCLabelBMFont::create("Click to Buy - 1000 diamond shards", "chatFont.fnt");
    priceLabel->setPosition(windowSize.width / 2, windowSize.height - 185);
    priceLabel->setScale(0.7f);
    priceLabel->setID("simple-player-price-label");
    this->addChild(priceLabel);
    
    // Check purchased items status and update UI
    checkPurchasedItems();

    return true;
}

void ZenGardenShopLayer::updateIconsLabel(float dt)
{
    // Get the local time
    std::time_t currentTime = std::time(nullptr);
    std::tm local_tm = *std::localtime(&currentTime);

    // Set the time to tomorrow at midnight
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    local_tm.tm_mday += 1;

    // Calculate the time until midnight
    std::time_t midnightTime = std::mktime(&local_tm);
    std::time_t remainingSeconds = midnightTime - currentTime;

    // Convert difference to hours, minutes, and seconds
    int hours = remainingSeconds / 3600;
    int minutes = (remainingSeconds % 3600) / 60;
    int seconds = remainingSeconds % 60;

    // Update the label text
    ZenGardenShopLayer::m_iconsLabel->setString((
                                                    std::string("Icons (restocks in ") + (hours < 10 ? "0" : "") + std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds) + ")")
                                                    .c_str());

    // Update the diamond shards label to keep it in sync with saved value
    updateDiamondShardsLabel();
}

void ZenGardenShopLayer::updateDiamondShardsLabel()
{
    if (m_diamondShardsLabel)
    {
        int currentShards = Mod::get()->getSavedValue<int>("money", 0);
        m_diamondShardsLabel->setString(std::to_string(currentShards).c_str());
    }
}

void ZenGardenShopLayer::checkForRestock()
{
    // Get current time
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm now_tm = *std::localtime(&now);
    
    // Get the last restock time
    time_t lastRestockTime = Mod::get()->getSavedValue<time_t>("shop_last_restock", 0);
    
    // If this is the first time, set the last restock time to now
    if (lastRestockTime == 0) {
        m_lastRestockTime = now;
        Mod::get()->setSavedValue<time_t>("shop_last_restock", now);
        return;
    }
    
    m_lastRestockTime = lastRestockTime;
    std::tm last_tm = *std::localtime(&m_lastRestockTime);
    
    // Check if it's a new day by comparing date components
    int now_day = now_tm.tm_mday;
    int now_month = now_tm.tm_mon + 1;
    int now_year = now_tm.tm_year + 1900;
    
    int last_day = last_tm.tm_mday;
    int last_month = last_tm.tm_mon + 1;
    int last_year = last_tm.tm_year + 1900;
    
    // If date components are different, it's a new day
    if (now_day != last_day || now_month != last_month || now_year != last_year) {
        // It's a new day, need to restock
        m_shouldRestock = true;
        
        // Call restockItems immediately
        restockItems();
    } else {
        m_shouldRestock = false;
    }
}

void ZenGardenShopLayer::onBuySimplePlayer(CCObject* sender)
{
    // Get the slot from the menu item's tag
    auto menuItem = dynamic_cast<CCMenuItemSpriteExtra*>(sender);
    if (!menuItem) return;
    
    int slotToPurchase = menuItem->getTag();
    
    // Check if this slot is already purchased
    if (m_itemPurchased[slotToPurchase]) {
        FLAlertLayer::create(
            "Already Purchased",
            "This Mystery Player has already been sold out!",
            "OK"
        )->show();
        return;
    }
    
    int cost = 1000;
    int currentShards = Mod::get()->getSavedValue<int>("money", 0);
    
    if (currentShards < cost) {
        FLAlertLayer::create(
            "Insufficient Funds",
            "You need 1000 diamond shards to purchase a <cr>mystery player</c>!",
            "OK"
        )->show();
        return;
    }
    
    // Create popup for confirmation
    geode::createQuickPopup(
        "Buy Mystery Player",
        "Are you sure you want to buy a <cr>mystery player</c> for 1000 diamond shards?",
        "Yes", "No",
        [this, cost, slotToPurchase](FLAlertLayer*, bool btn2)
        {
            if (!btn2)
            {
                // Check if there's room for a new player
                bool success = ZenGardenLayer::addRandomSimplePlayer();
                
                if (!success) {
                    FLAlertLayer::create(
                        "No Room",
                        "You don't have any free spots for a new player in your garden!",
                        "OK"
                    )->show();
                    return;
                }
                
                // Get current values
                int currentShards = Mod::get()->getSavedValue<int>("money", 0);
                
                // Update values
                currentShards -= cost;
                
                // Save the updated values
                Mod::get()->setSavedValue<int>("money", currentShards);
                
                // Update diamond shards count in ZenGardenLayer
                ZenGardenLayer::m_diamondShards = currentShards;
                
                // Mark this slot as purchased
                m_itemPurchased[slotToPurchase] = true;
                
                // Update UI to reflect the purchase
                updateItemAvailability();
                
                // Update the diamond shards label
                updateDiamondShardsLabel();
                
                // Update ZenGardenLayer labels if it exists
                updateZenGardenLayerLabels();
                
                FLAlertLayer::create(
                    "Purchase Successful",
                    "You have purchased a <cr>mystery player</c>! Return to your garden to see it!",
                    "OK"
                )->show();
            }
        });
}

void ZenGardenShopLayer::updateZenGardenLayerLabels()
{
    // Check if ZenGardenLayer is open and update its labels
    if (ZenGardenLayer::m_starsLabel != nullptr)
    {
        ZenGardenLayer::m_starsLabel->setString(std::to_string(ZenGardenLayer::m_starCount).c_str());
    }

    if (ZenGardenLayer::m_moonsLabel != nullptr)
    {
        ZenGardenLayer::m_moonsLabel->setString(std::to_string(ZenGardenLayer::m_moonCount).c_str());
    }

    if (ZenGardenLayer::m_diamondsLabel != nullptr)
    {
        ZenGardenLayer::m_diamondsLabel->setString(std::to_string(ZenGardenLayer::m_diamondCount).c_str());
    }

    if (ZenGardenLayer::m_diamondShardsLabel != nullptr)
    {
        ZenGardenLayer::m_diamondShardsLabel->setString(std::to_string(ZenGardenLayer::m_diamondShards).c_str());
    }
}

void ZenGardenShopLayer::checkPurchasedItems()
{
    // Find the player icons and question marks
    auto playerIcon1 = this->getChildByID("simple-player-icon-1");
    auto playerIcon2 = this->getChildByID("simple-player-icon-2");
    auto questionMark1 = this->getChildByID("question-mark-1");
    auto questionMark2 = this->getChildByID("question-mark-2");
    auto iconSlot1 = this->getChildByID("icon-slot-1");
    auto iconSlot2 = this->getChildByID("icon-slot-2");
    
    // Update visibility based on purchase state
    if (playerIcon1 && questionMark1) {
        if (m_itemPurchased[0]) {
            playerIcon1->setVisible(false);
            questionMark1->setVisible(false);
            
            // Add "Sold Out" text over the slot
            if (!this->getChildByID("sold-out-1")) {
                auto soldOut = CCLabelBMFont::create("SOLD", "goldFont.fnt");
                soldOut->setScale(0.6f);
                soldOut->setColor(ccRED);
                soldOut->setPosition(playerIcon1->getPosition());
                soldOut->setID("sold-out-1");
                this->addChild(soldOut, 10);
            }
        } else {
            playerIcon1->setVisible(true);
            questionMark1->setVisible(true);
            
            // Remove "Sold Out" text if it exists
            auto soldOut = this->getChildByID("sold-out-1");
            if (soldOut) {
                this->removeChild(soldOut);
            }
        }
    }
    
    if (playerIcon2 && questionMark2) {
        if (m_itemPurchased[1]) {
            playerIcon2->setVisible(false);
            questionMark2->setVisible(false);
            
            // Add "Sold Out" text over the slot
            if (!this->getChildByID("sold-out-2")) {
                auto soldOut = CCLabelBMFont::create("SOLD", "goldFont.fnt");
                soldOut->setScale(0.6f);
                soldOut->setColor(ccRED);
                soldOut->setPosition(playerIcon2->getPosition());
                soldOut->setID("sold-out-2");
                this->addChild(soldOut, 10);
            }
        } else {
            playerIcon2->setVisible(true);
            questionMark2->setVisible(true);
            
            // Remove "Sold Out" text if it exists
            auto soldOut = this->getChildByID("sold-out-2");
            if (soldOut) {
                this->removeChild(soldOut);
            }
        }
    }
    
    // Update player icon menu items
    auto playerMenu = this->getChildByID("player-icons-menu");
    if (playerMenu) {
        // Update player icon 1 status
        auto playerItem1 = static_cast<CCMenuItem*>(playerMenu->getChildByID("simple-player-icon-1"));
        if (playerItem1) {
            playerItem1->setEnabled(!m_itemPurchased[0]);
        }
        
        // Update player icon 2 status
        auto playerItem2 = static_cast<CCMenuItem*>(playerMenu->getChildByID("simple-player-icon-2"));
        if (playerItem2) {
            playerItem2->setEnabled(!m_itemPurchased[1]);
        }
        
        // Update price label
        auto priceLabel = this->getChildByID("simple-player-price-label");
        if (priceLabel && dynamic_cast<CCLabelBMFont*>(priceLabel)) {
            if (m_itemPurchased[0] && m_itemPurchased[1]) {
                static_cast<CCLabelBMFont*>(priceLabel)->setString("Sold Out - Restocks Tomorrow");
                static_cast<CCLabelBMFont*>(priceLabel)->setColor(ccRED);
            } else {
                static_cast<CCLabelBMFont*>(priceLabel)->setString("Click to Buy - 1000 diamond shards");
                static_cast<CCLabelBMFont*>(priceLabel)->setColor(ccWHITE);
            }
        }
    }
}

void ZenGardenShopLayer::updateItemAvailability()
{
    // Update visibility of items based on purchase state
    checkPurchasedItems();
    
    // Save purchase state
    Mod::get()->setSavedValue<bool>("shop_item_1_purchased", m_itemPurchased[0]);
    Mod::get()->setSavedValue<bool>("shop_item_2_purchased", m_itemPurchased[1]);
    Mod::get()->setSavedValue<time_t>("shop_last_restock", m_lastRestockTime);
}

void ZenGardenShopLayer::restockItems()
{
    // Restock all items
    m_itemPurchased[0] = false;
    m_itemPurchased[1] = false;
    
    // Update last restock time
    m_lastRestockTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    
    // Remove any "SOLD" labels
    auto soldOut1 = this->getChildByID("sold-out-1");
    if (soldOut1) {
        this->removeChild(soldOut1);
    }
    
    auto soldOut2 = this->getChildByID("sold-out-2");
    if (soldOut2) {
        this->removeChild(soldOut2);
    }
    
    // Reset the player icon menu items and price label
    auto playerMenu = this->getChildByID("player-icons-menu");
    if (playerMenu) {
        // Enable player icon 1
        auto playerItem1 = static_cast<CCMenuItem*>(playerMenu->getChildByID("simple-player-icon-1"));
        if (playerItem1) {
            playerItem1->setEnabled(true);
        }
        
        // Enable player icon 2
        auto playerItem2 = static_cast<CCMenuItem*>(playerMenu->getChildByID("simple-player-icon-2"));
        if (playerItem2) {
            playerItem2->setEnabled(true);
        }
    }
    
    auto priceLabel = this->getChildByID("simple-player-price-label");
    if (priceLabel && dynamic_cast<CCLabelBMFont*>(priceLabel)) {
        static_cast<CCLabelBMFont*>(priceLabel)->setString("Click to Buy - 1000 diamond shards");
        static_cast<CCLabelBMFont*>(priceLabel)->setColor(ccWHITE);
    }
    
    // Show a notification about the restock
    geode::Notification::create("Shop Restocked!", geode::NotificationIcon::Success, 3.0f)->show();
    
    // Save state
    updateItemAvailability();
}