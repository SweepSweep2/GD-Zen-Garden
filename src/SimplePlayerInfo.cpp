#include "SimplePlayerInfo.hpp"
#include <Geode/Geode.hpp>
#include <Geode/binding/GameManager.hpp>
#include <sstream>
#include <cstdlib>

using namespace geode::prelude;

SimplePlayerInfo *SimplePlayerInfo::create(SimplePlayer *player, int slotIndex)
{
    auto ret = new SimplePlayerInfo();

    // Set slot index BEFORE init so setup()
    if (ret)
        ret->m_slotIndex = slotIndex;

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

    // Load per-slot maturity data if slot is valid, else fallback to global
    if (m_slotIndex >= 0)
    {
        m_maturityLevel = Mod::get()->getSavedValue<int>("player_maturity_" + std::to_string(m_slotIndex), 0);
        m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed_" + std::to_string(m_slotIndex), 0);
    }
    else
    {
        m_maturityLevel = Mod::get()->getSavedValue<int>("player_maturity", 0);
        m_orbsFeeded = Mod::get()->getSavedValue<int>("player_orbs_fed", 0);
    }

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

    // Display the clicked player's icon/colors.
    auto displayPlayer = SimplePlayer::create(1);
    int iconId = 1;
    int color1Idx = 1;
    int color2Idx = 1;
    // Try to read icon id stored on the clicked SimplePlayer
    if (m_displayedPlayer)
    {
        iconId = m_displayedPlayer->getTag();
        if (iconId <= 0)
            iconId = 1;
    }
    // If tag wasn't set for some reason, use saved slot data
    if (m_slotIndex >= 0)
    {
        std::string key = "player_" + std::to_string(m_slotIndex);
        std::string playerInfo = Mod::get()->getSavedValue<std::string>(key, "");
        if (!playerInfo.empty())
        {
            std::stringstream ss(playerInfo);
            std::string tok;
            std::vector<int> vals;
            while (std::getline(ss, tok, ','))
            {
                bool ok = true;
                if (tok.empty())
                    ok = false;
                for (char ch : tok)
                {
                    if (!(ch == '-' || (ch >= '0' && ch <= '9')))
                    {
                        ok = false;
                        break;
                    }
                }
                if (ok)
                {
                    // std::stoi may still throw on overflow; guard by using std::strtol
                    char *endp = nullptr;
                    long v = std::strtol(tok.c_str(), &endp, 10);
                    if (endp && *endp == '\0')
                    {
                        vals.push_back(static_cast<int>(v));
                    }
                }
            }
            if (vals.size() >= 3)
            {
                if (iconId == 1)
                    iconId = vals[0];
                color1Idx = vals[1];
                color2Idx = vals[2];
            }
        }
    }
    displayPlayer->updatePlayerFrame(iconId, static_cast<IconType>(0));
    // Apply the saved colors (these are what the garden used for the clicked player)
    auto gm = GameManager::sharedState();
    displayPlayer->setColors(gm->colorForIdx(color1Idx), gm->colorForIdx(color2Idx));
    displayPlayer->setScale(1.5f);
    displayPlayer->setPosition(ccp(125, 100));
    this->m_mainLayer->addChild(displayPlayer);

    // Name input above the icon using geode::TextInput
    m_nameInput = geode::TextInput::create(180.f, "Enter name...", "bigFont.fnt");
    if (m_nameInput)
    {
        // Allow typical name characters
        m_nameInput->setCommonFilter(CommonFilter::Alphabetic);
        m_nameInput->setMaxCharCount(20);
        m_nameInput->setTextAlign(TextInputAlign::Center);
        // Position above the displayed player icon
        m_nameInput->setPosition({125.f, 150.f});
        // Load saved name if any
        std::string savedName;
        if (m_slotIndex >= 0)
        {
            savedName = Mod::get()->getSavedValue<std::string>(
                "player_name_" + std::to_string(m_slotIndex), "");
        }
        else
        {
            savedName = Mod::get()->getSavedValue<std::string>("player_name", "");
        }
        if (!savedName.empty())
        {
            m_nameInput->setString(savedName, false);
        }
        // Save on change
        m_nameInput->setCallback([this](std::string const &value)
                                 {
            if (m_slotIndex >= 0) {
                Mod::get()->setSavedValue("player_name_" + std::to_string(m_slotIndex), value);
            } else {
                Mod::get()->setSavedValue("player_name", value);
            } });
        this->m_mainLayer->addChild(m_nameInput);
    }

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
