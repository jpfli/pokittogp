#pragma once

#include "main.h"
#include "ship.h"
#include "menu.h"

class CPlayerShip : public CShip
{
public:
    CPlayerShip();
    virtual void Reset();
    virtual void Update();
    void HandleGameKeys();
    void HandleRaceStartingAndEnding( bool isOnStartingGrid );
    void FindActiveWaypoint();
    void CalculateRank();

public: // From CShip
    virtual void SetImpulse( fix16_t fxImpulseAngle )
    {
        CShip::SetImpulse(fxImpulseAngle);
        m_bitmap = billboard_object_bitmaps[2];  // red car
    }

public:
    uint32_t m_final_lap_time_ms;
    uint32_t m_start_ms;
    int m_tonefreq;
    fix16_t m_fxCameraBehindPlayerTarget;
    fix16_t m_fxCameraBehindPlayerCurrent;
    int32_t m_currentRank;
    uint32_t m_current_lap_time_ms;
    CMenu::MenuMode m_requestedMenuMode;
    int8_t m_lastCheckedWPIndex;
    uint32_t m_activeWaypointFoundTimeInMs;
};
