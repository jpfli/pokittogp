#pragma once

#include "main.h"
#include "animation.h"

class CShip : public CObject3d, public CAnimValueCallback
{
public:
    enum TileType {
        enumTrackTile = 0,
        enumTerrainTile = 1,
        enumEdgeTile = 2
    };

public:
    CShip();

    static void UpdateStarted();

    virtual void Update();
    virtual void Reset();
    virtual void SetImpulse( fix16_t fxImpulseAngle );

    void AIUpdate(TileType tileType);

    // val: 0..100
    void SetAIAggressiveness(int8_t val) { m_fxCoef_aiAggr = fix16_one*(10000/(400-3*val))/100; }

public:
    fix16_t m_fxAngle;
    int32_t m_activeWaypointIndex;
    int32_t m_activeLapNum;
    LapTimingState m_lapTimingState;

    fix16_t m_fxCoef_aiAggr;

    // Impact
    fix16_t m_fxImpulseAcc;
    fix16_t m_fxImpulseAngle;

    CAnimValue* m_jumpAnimValue;
    CAnimValue* m_boosterAnimValue;


// Physics based driving model:
public:
    fix16_t PhysicsDeltaTime() const { return m_fxDeltaTime; }

    fix16_t Yaw() const { return m_fxAngle; }
    void SetYaw(fix16_t angle) { m_fxAngle = angle; }

    fix16_t VelocityX() const { return ScaleToGameCoords(m_fxVel_x); }
    fix16_t VelocityY() const { return ScaleToGameCoords(m_fxVel_y); }
    void SetVelocity(fix16_t x, fix16_t y) { m_fxVel_x = ScaleFromGameCoords(x); m_fxVel_y = ScaleFromGameCoords(y); }

    bool IsSliding() const { return m_isSliding; }
    bool IsGrounded() const { return m_isGrounded; }
    bool IsBoosterActive() const { return m_booster; }

    void StartBooster();
    void StartJump();

    int8_t Steering() const { return m_steering; }
    void SetSteering(int8_t val) { if(val > 100) m_steering = 100; else if(val < -100) m_steering = 100; else {m_steering = val; } }
    uint8_t Throttle() const { return m_throttle; }
    void SetThrottle(uint8_t val) { if(val > 100) m_throttle = 100; else {m_throttle = val; } }
    uint8_t Braking() const { return m_braking; }
    void SetBraking(uint8_t val) { if(val > 100) m_braking = 100; else {m_braking = val; } }

    void PhysicsUpdate(TileType tileType);

    static void SetPhysicsParameters(TileType type, fix16_t fxStaticFriction, fix16_t fxKineticFriction, uint16_t tractionConstant, fix16_t fxRollingResistance);
    static void SetShipParameters(fix16_t fxMaxSteering, fix16_t fxMaxThrust, fix16_t fxMaxBraking);
    static void ResetDefaultPhysicsParameters();
    static void ResetDefaultShipParameters();

    static fix16_t CalculateBrakingDistance(fix16_t fxSpeed, TileType tileType);

private:
    constexpr static fix16_t m_fxNpcSteeringDeadzone = fix16_pi*2.0/180;
    constexpr static fix16_t m_fxCoef_npcNominalSpeed = 288.0*fix16_one/100;

    // A more accurate fix16 sin function
    static fix16_t sin_taylor(fix16_t radians ) {
        // Limit angle to -90..270
        while( -(fix16_pi>>1) > radians ) {
            radians += fix16_pi<<1;
        }
        while( (3*fix16_pi)>>1 <= radians ) {
            radians -= fix16_pi<<1;
        }
        // Mirror angles larger than 90 degrees
        if( fix16_pi>>1 < radians ) {
            radians = fix16_pi-radians;
        }

        // For angles close to -90 and 90 degrees use cosine approximation instead
        const fix16_t LIMIT = 60222; // 52.65 degrees
        if( LIMIT <= radians ) {
            radians -= fix16_pi>>1;
            const fix16_t angle_sq = fix16_mul( radians, radians );
            return fix16_one-fix16_mul( angle_sq, (fix16_one>>1)-fix16_mul( angle_sq, 2731 ) );
        }
        if( -LIMIT >= radians ) {
            radians += (fix16_pi>>1);
            const fix16_t angle_sq = fix16_mul( radians, radians );
            return fix16_mul( angle_sq, (fix16_one>>1)-fix16_mul( angle_sq, 2731 ) )-fix16_one;
        }
        // For all other angle use sine approximation
        const fix16_t angle_sq = fix16_mul( radians, radians );
        return fix16_mul( radians, fix16_one-fix16_mul( angle_sq, 10923-fix16_mul( angle_sq, 546 ) ) );
    }

    inline static fix16_t cos_taylor(fix16_t radians) { return sin_taylor(radians+(fix16_pi>>1)); }

    inline static fix16_t ScaleFromGameCoords(fix16_t val) { return val>>4; }
    inline static fix16_t ScaleToGameCoords(fix16_t val) { return val<<4; }

public: // From CAnimValueCallback

    // When this is called the CAnimValue is finished and will be used for other tasks. The caller should either
    // set a pointer to the CAnimValue object to NULL, or start another animation on the object.
    // The parameter can be used e.g. for chained animations to indicate the state.
    void Finished( int32_t par );

private:
    struct PhysicsParameters {
        fix16_t fxDDVel_fs;
        fix16_t fxDDVel_fk;
        uint16_t traction;
        fix16_t fxCoef_rr;
    };

    struct ShipParameters {
        fix16_t fxCoef_turn;
        fix16_t fxCoef_thrust;
        fix16_t fxCoef_brake;
    };

    // Physics state variables
    fix16_t m_fxVel_x;
    fix16_t m_fxVel_y;
    bool m_isSliding;
    bool m_isGrounded;

    // Ship controls
    int8_t m_steering; // negative to steer right, positive to steer left [-100..100]
    uint8_t m_throttle; // amount of throttling [0..100]
    uint8_t m_braking; // amount of braking [0..100]
    bool m_booster; // booster on/off

    static PhysicsParameters m_physParmsArray[3];
    static ShipParameters m_shipParms;
    static fix16_t m_fxDeltaTime; //time between updates
    static uint32_t m_prevFrameTimeInMs;

    // Physics constants
    constexpr static fix16_t m_fxGravity = 9.81*fix16_one;
    constexpr static uint16_t m_mass = 650;
    // 1/2 * [air density] * [ship cross sectional area] * [air resistance]
    constexpr static fix16_t m_fxFactor_drag = 1.29*2.2*0.30*fix16_one/2;
};

