#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include "SimplePlayerInfo.hpp"

using namespace geode::prelude;
using namespace cocos2d;

class ZenGardenLayer : public cocos2d::CCLayer
{
public:
    static ZenGardenLayer *create();
    bool init() override;
    void onBack(CCObject *sender);
    void onShop(CCObject *sender);
    void keyBackClicked() override;
    void onFeedOrbs(CCObject *sender);
    void onFeedStars(CCObject *sender);
    void onFeedMoons(CCObject *sender);
    void onFeedDiamonds(CCObject *sender);
    void onSimplePlayerClicked(CCObject *sender);
    void showBronzeCoinReward();
    void showSilverCoinReward();
    void showGoldCoinReward();
    void flashShards();
    void removeCoinSprite(CCNode* sender);
    void tryEmitCoins();
    void update(float dt);
    
    // Maturity system methods
    void initializePlayerMaturity();
    void updatePlayerMaturityVisuals();
    void handlePlayerGrowth();
    bool canFeedOrb();
    void displayOrbCooldownMessage();
    void displayRequirementSprite();
    void hideRequirementSprite();
    
    // Reset progress functionality
    void onResetProgress(CCObject* sender);
    void confirmResetProgress();
    
    // Cheat functionality
    void cheat();
    void keyDown(enumKeyCodes key) override;

    int m_selectedItem = 0;
    int m_starCount = 2;
    int m_moonCount = 3;
    int m_diamondCount = 4;
    int m_diamondShards = 0;
    CCSprite *m_orbsHoverSprite;
    CCSprite *m_starsHoverSprite;
    CCSprite *m_moonsHoverSprite;
    CCSprite *m_diamondsHoverSprite;
    CCLabelBMFont *m_starsLabel;
    CCLabelBMFont *m_moonsLabel;
    CCLabelBMFont *m_diamondsLabel;
    CCLabelBMFont *m_diamondShardsLabel;
    CCSprite *m_diamondCurrencyIcon;
    SimplePlayer *m_simplePlayer;
    
    // Maturity system variables
    int m_maturityLevel = 0; // 0-5, with 5 being fully mature
    int m_orbsFeeded = 0; // Track orbs fed to the player
    int64_t m_lastOrbFeedTime = 0; // When the player was last fed an orb (stored as system time in seconds)
    CCSprite *m_requirementSprite = nullptr; // Visual indicator for what food the player needs
    CCLabelBMFont *m_requirementLabel = nullptr; // Label for orbs progress (X/5)
};