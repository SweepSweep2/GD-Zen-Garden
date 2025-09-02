#include "SimplePlayerInfo.hpp"
#include <Geode/Geode.hpp>

SimplePlayerInfo *SimplePlayerInfo::create(SimplePlayer *player)
{
    auto ret = new SimplePlayerInfo();

    if (ret && ret->initAnchored(250.0f, 200.0f, player))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SimplePlayerInfo::setup(SimplePlayer *player)
{
    m_displayedPlayer = player;

    // Initialize maturity data
    m_maturityLevel = Mod::get()->getSavedValue<int>("player_maturity", 0);
    m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed", 0);

    // Determine current requirement based on maturity level
    if (m_maturityLevel < 1)
    {
        m_currentRequirement = GrowthRequirement::ORB;
    }
    else if (m_maturityLevel < 2)
    {
        m_currentRequirement = GrowthRequirement::STAR;
    }
    else if (m_maturityLevel < 3)
    {
        m_currentRequirement = GrowthRequirement::MOON;
    }
    else if (m_maturityLevel < 4)
    {
        m_currentRequirement = GrowthRequirement::DIAMOND;
    }
    else
    {
        m_currentRequirement = GrowthRequirement::NONE;
    }

    this->setTitle("Player Info");

    // Display player with current icon
    auto displayPlayer = SimplePlayer::create(1); // Fixed cube icon
    displayPlayer->setScale(1.5f);
    displayPlayer->setPosition(ccp(125, 100));
    this->m_mainLayer->addChild(displayPlayer);

    updateMaturityInfo();

    return true;
}

void SimplePlayerInfo::updateMaturityInfo()
{
    // Add maturity level display
    std::string maturityText;
    ccColor3B textColor;

    switch (m_maturityLevel)
    {
    case 0:
        maturityText = "Maturity: Level 0";
        textColor = {255, 204, 153}; // Light orange for baby
        break;
    case 1:
        maturityText = "Maturity: Level 1";
        textColor = {153, 255, 153}; // Light green for young
        break;
    case 2:
        maturityText = "Maturity: Level 2";
        textColor = {153, 204, 255}; // Light blue for teen
        break;
    case 3:
        maturityText = "Maturity: Level 3";
        textColor = {204, 153, 255}; // Light purple for adult
        break;
    case 4:
        maturityText = "Maturity: Level 4";
        textColor = {255, 153, 204}; // Pink for elder
        break;
    default:
        maturityText = "Maturity: Level 5";
        textColor = {255, 215, 0}; // Gold for fully mature
    }

    auto maturityLabel = CCLabelBMFont::create(maturityText.c_str(), "bigFont.fnt");
    maturityLabel->setScale(0.5f);
    maturityLabel->setColor(textColor);
    maturityLabel->setPosition(ccp(125, 50));
    this->m_mainLayer->addChild(maturityLabel);

    // Display growth requirement
    std::string reqText;

    if (m_maturityLevel < 5)
    {
        switch (m_currentRequirement)
        {
        case GrowthRequirement::ORB:
            reqText = "Needs: Orbs (" + std::to_string(m_orbsFeeded) + "/5)";
            break;
        case GrowthRequirement::STAR:
            reqText = "Needs: Stars";
            break;
        case GrowthRequirement::MOON:
            reqText = "Needs: Moons";
            break;
        case GrowthRequirement::DIAMOND:
            reqText = "Needs: Diamonds";
            break;
        default:
            reqText = "Fully Grown!";
        }
    }
    else
    {
        reqText = "Fully Grown!";
    }

    auto requirementLabel = CCLabelBMFont::create(reqText.c_str(), "bigFont.fnt");
    requirementLabel->setScale(0.45f);
    requirementLabel->setPosition(ccp(125, 25));
    this->m_mainLayer->addChild(requirementLabel);
}
