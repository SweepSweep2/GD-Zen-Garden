#pragma once
#include <cocos2d.h>

#include <Geode/Geode.hpp>

#include "ZenGardenLayer.hpp"

using namespace geode::prelude;
using namespace cocos2d;

class ZenGardenShopLayer : public cocos2d::CCLayer {
     public:
      static ZenGardenShopLayer* create();
      bool init() override;
      void onBack(CCObject* sender);
      void onBuyStars(CCObject* sender);
      void onBuyMoons(CCObject* sender);
      void onBuyDiamonds(CCObject* sender);
      void onBuySimplePlayer(CCObject* sender);
      void updateIconsLabel(float dt);
      void updateDiamondShardsLabel();
      void updateZenGardenLayerLabels();
      void checkForRestock();
      void checkPurchasedItems();
      void updateItemAvailability();
      void restockItems();
      void keyDown(enumKeyCodes key) override;

      CCLabelBMFont* m_iconsLabel;
      CCLabelBMFont* m_diamondShardsLabel;
      CCLabelBMFont* m_starsLabel;
      CCLabelBMFont* m_moonsLabel;
      CCLabelBMFont* m_diamondsLabel;
      bool m_itemPurchased[4] = {false, false, false, false};  // Track if each item slot has been purchased
      time_t m_lastRestockTime = 0;                            // Last time items were restocked
      bool m_shouldRestock = false;                            // Flag indicating if restock is needed
};