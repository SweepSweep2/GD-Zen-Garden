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
    void onEnter() override;
    void onBack(CCObject *sender);
    void onShop(CCObject *sender);
    void keyBackClicked() override;
    void onFeedOrbs(CCObject *sender);
    void onFeedStars(CCObject *sender);
    void onFeedMoons(CCObject *sender);
    void onFeedDiamonds(CCObject *sender);
    void onSimplePlayerClicked(CCObject *sender);
    void showBronzeCoinReward(int pos);
    void showSilverCoinReward(int pos);
    void showGoldCoinReward(int pos);
    void flashShards();
    void removeCoinSprite(CCNode* sender);
    void tryEmitCoins(int pos);
    void update(float dt);
    
    // Maturity system methods
    void initializePlayerMaturity();
    void updatePlayerMaturityVisuals();
    void handlePlayerGrowth();
    bool canFeedOrb();
    void displayOrbCooldownMessage();
    // Per-slot variants (operate on the slot index directly, independent of active selection)
    void updatePlayerMaturityVisualsForPos(int pos);
    void handlePlayerGrowthForPos(int pos);
    bool canFeedOrbForPos(int pos);
    void displayOrbCooldownMessageForPos(int pos);
    SimplePlayer* getPlayerNodeAtPos(int pos);
    // Show/remove requirement overlays for all players
    void displayRequirementSprite();
    void hideRequirementSprite();
    
    // Reset progress functionality
    void onResetProgress(CCObject* sender);
    void confirmResetProgress();
    
    // Cheat functionality
    void cheat();
    void keyDown(enumKeyCodes key) override;

    // SimplePlayer management
    static bool addRandomSimplePlayer();
    static CCArray* getOccupiedPositions();
    void reloadGardenFromSaves();
    void updateSlotRequirementUI(int pos);

    // Active player tracking (slot index 0-7), -1 when none
    int m_activePos = -1;

    int m_selectedItem = 0;
    static int m_starCount;
    static int m_moonCount;
    static int m_diamondCount;
    static int m_diamondShards;
    CCSprite *m_orbsHoverSprite;
    CCSprite *m_starsHoverSprite;
    CCSprite *m_moonsHoverSprite;
    CCSprite *m_diamondsHoverSprite;
    static CCLabelBMFont *m_starsLabel;
    static CCLabelBMFont *m_moonsLabel;
    static CCLabelBMFont *m_diamondsLabel;
    static CCLabelBMFont *m_diamondShardsLabel;
    CCSprite *m_diamondCurrencyIcon;
    SimplePlayer *m_simplePlayer;
    
    // Maturity system variables
    int m_maturityLevel = 0; // 0-5, with 5 being fully mature
    int m_orbsFeeded = 0; // Track orbs fed to the player
    int64_t m_lastOrbFeedTime = 0; // When the player was last fed an orb (stored as system time in seconds)
    // Legacy single references (kept for compatibility but unused by per-slot overlays)
    CCSprite *m_requirementSprite = nullptr;
    CCLabelBMFont *m_requirementLabel = nullptr;
};