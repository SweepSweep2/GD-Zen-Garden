#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>

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
    void updateIconsLabel(float dt);
    CCLabelBMFont *m_iconsLabel;
};