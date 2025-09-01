#pragma once
#include <Geode/Geode.hpp>
#include <cocos2d.h>

using namespace geode::prelude;
using namespace cocos2d;

class ZenGardenLayer : public cocos2d::CCLayer
{
public:
    static ZenGardenLayer *create();
    bool init() override;
    void onBack(CCObject *sender);
    void keyBackClicked() override;
    void onFeedOrbs(CCObject *sender);
    void onFeedStars(CCObject *sender);
    void onFeedMoons(CCObject *sender);
    void onFeedDiamonds(CCObject *sender);
    void onSimplePlayerClicked(CCObject *sender);
    void showCoinReward();
    void flashMoneyGreen();
    void removeCoinSprite(CCNode* sender);
    void update(float dt);

    int m_selectedItem = 0;
    int m_starCount = 2;
    int m_moonCount = 3;
    int m_diamondCount = 4;
    int m_money = 1000;
    CCSprite *m_orbsHoverSprite;
    CCSprite *m_starsHoverSprite;
    CCSprite *m_moonsHoverSprite;
    CCSprite *m_diamondsHoverSprite;
    CCLabelBMFont *m_starsLabel;
    CCLabelBMFont *m_moonsLabel;
    CCLabelBMFont *m_diamondsLabel;
    CCLabelBMFont *m_moneyLabel;
    SimplePlayer *m_simplePlayer;
};