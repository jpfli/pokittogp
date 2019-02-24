#pragma once

#include "ship.h"
#include "main.h"

class CNpcShip : public CShip
{
public:
    CNpcShip();
    virtual void Update();
    virtual void Reset();
    virtual void SetImpulse( fix16_t fxImpulseAngle );

public:
    fix16_t m_fxVel;
    fix16_t m_fxAcc;
    fix16_t m_fxDeacc;
    fix16_t m_fxAngle;
    fix16_t m_fxRotVel;
    fix16_t m_fxMaxSpeed;
    fix16_t m_fxCornerSpeed1;
    fix16_t m_fxCornerSpeed2;
    int32_t m_activeWaypointIndex;
    fix16_t m_fxWaypointTargetSpeed;
    //fix16_t m_fxLastDistanceToWaypoint;
    bool m_isPlayer;
    int32_t m_activeLapNum;
    LapTimingState m_lapTimingState;
};
