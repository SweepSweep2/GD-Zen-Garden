#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include "ZenGardenLayer.hpp"

using namespace geode::prelude;
using namespace cocos2d;

class ZenGardenShopLayer : public cocos2d::CCLayer {
public:
    static ZenGardenShopLayer *create();
    bool init() override;
    void onBack(CCObject *sender);
    void onBuyStars(CCObject *sender);
    void onBuyMoons(CCObject *sender);
    void onBuyDiamonds(CCObject *sender);
    void onBuySimplePlayer(CCObject *sender);
    void updateIconsLabel(float dt);
    void updateDiamondShardsLabel();
    static void updateZenGardenLayerLabels();
    void checkForRestock();
    void checkPurchasedItems();
    void updateItemAvailability();
    void restockItems();
    
    CCLabelBMFont *m_iconsLabel;
    CCLabelBMFont *m_diamondShardsLabel;
    bool m_itemPurchased[3] = {false, false, false}; // Track if each item slot has been purchased
    time_t m_lastRestockTime = 0; // Last time items were restocked
    bool m_shouldRestock = false; // Flag indicating if restock is needed
};