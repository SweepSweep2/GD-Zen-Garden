#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/binding/SimplePlayer.hpp>

using namespace geode::prelude;

// Define growth requirement types
enum class GrowthRequirement {
    NONE = 0,
    ORB = 1,
    STAR = 2,
    MOON = 3,
    DIAMOND = 4
};

class SimplePlayerInfo : public geode::Popup<SimplePlayer *>
{
protected:
    SimplePlayer *m_displayedPlayer;
    int m_slotIndex = -1; // Garden slot index for this player (0-7)
    int m_maturityLevel; // Current maturity level (0-5)
    int m_orbsFeeded; // Number of orbs fed to this player so far
    GrowthRequirement m_currentRequirement; // Current growth requirement
    geode::TextInput* m_nameInput = nullptr; // Name input field
    
    bool setup(SimplePlayer *player) override;
    void updateMaturityInfo(); // Updates the display with maturity information
    
public:
    static SimplePlayerInfo *create(SimplePlayer *player, int slotIndex);
    
    // Getters and setters
    int getMaturityLevel() const { return m_maturityLevel; }
    void setMaturityLevel(int level) { m_maturityLevel = level; }
    
    int getOrbsFeeded() const { return m_orbsFeeded; }
    void setOrbsFeeded(int orbs) { m_orbsFeeded = orbs; }
    
    GrowthRequirement getCurrentRequirement() const { return m_currentRequirement; }
    void setCurrentRequirement(GrowthRequirement req) { m_currentRequirement = req; }
};
