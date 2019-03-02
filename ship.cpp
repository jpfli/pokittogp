#include "Pokitto.h"
#include "PokittoCookie.h"
#include "fix16.h"
#include "ship.h"

CShip::PhysicsParameters CShip::m_physParmsArray[3];
CShip::ShipParameters CShip::m_shipParms;

CShip::CShip() :
    m_fxAngle(0),
    m_activeLapNum(0),
    m_lapTimingState(enumReadyToStart),
    m_fxImpulseAcc(0),
    m_fxImpulseAngle(0),
    m_jumpAnimValue(nullptr),
    m_boosterAnimValue(nullptr),

    m_fxVel_x(0),
    m_fxVel_y(0),
    m_isSliding(false),
    m_isGrounded(true),
    m_steering(0),
    m_throttle(0),
    m_braking(0),
    m_booster(false)
{
}

void CShip::Update()
{
    // *** Check collision to road edges
    TileType tileType = enumTrackTile;
    uint8_t tileIndex = GetTileIndexCommon(fix16_to_int(m_fxX), fix16_to_int(m_fxY));
    if(tileIndex >= 1 && tileIndex <= 4) {
        tileType = enumEdgeTile;
    }
    else if((tileIndex >= 7 && tileIndex <= 10) || tileIndex == 0) {
        tileType = enumTerrainTile;
    }
    else if(tileIndex == 15) {
        // Booster.
        CShip::StartBooster();
    }
    // Jump disabled for AI.
    //else if(tileIndex == 16) {
    //    CShip::StartJump();
    //}

    // Handle lap starting and ending detection.
    bool isOnStartingGrid = ( tileIndex >= 11 && tileIndex <= 14);
    switch(m_lapTimingState)
    {
    case enumReadyToStart:
        if( isOnStartingGrid )
        {
            m_lapTimingState = enumStarted;
        }
        break;
    case enumStarted:
        if( ! isOnStartingGrid )
        {
            m_lapTimingState = enumOnTimedTrack;
        }
        break;
    case enumOnTimedTrack:
        if( isOnStartingGrid )
        {
            // Finished!
            m_lapTimingState = enumStarted;
            m_activeLapNum++;
        }
        break;
    case enumFinished:
        break;
    }

    // *** AI begin ***

    AIUpdate(tileType);

    // *** AI end ***


    // *** Physics start ***

    fix16_t fxVel_x0 = VelocityX();
    fix16_t fxVel_y0 = VelocityY();
    PhysicsUpdate(tileType);
    m_fxX += fix16_mul((fxVel_x0+VelocityX())>>1, DeltaTime());
    m_fxY += fix16_mul((fxVel_y0+VelocityY())>>1, DeltaTime());

    // *** Physics end ***


    if( m_fxImpulseAcc != 0 )
    {
        // Move ship along the impulse.
        fix16_t fxImpulseCos = fix16_cos( m_fxImpulseAngle );
        fix16_t fxImpulseSin = fix16_sin( m_fxImpulseAngle );
        m_fxX += fix16_mul(m_fxImpulseAcc, fxImpulseCos);
        m_fxY += fix16_mul(m_fxImpulseAcc, fxImpulseSin);

        // Reset impulse
        m_fxImpulseAcc = 0;
        m_fxImpulseAngle = 0;
    }
}

void CShip::SetImpulse( fix16_t fxImpulseAngle )
{
    m_fxImpulseAcc = fix16_one<<2;
    m_fxImpulseAngle = fxImpulseAngle; // -(fix16_pi>>2);
}

void CShip::Reset()
{
    m_fxAngle = fix16_pi>>1;
    m_activeWaypointIndex = 0;

    if( m_jumpAnimValue )
        m_jumpAnimValue->Reset();
    m_jumpAnimValue = NULL;
    if( m_boosterAnimValue )
        m_boosterAnimValue->Reset();
    m_boosterAnimValue = NULL;

    m_activeLapNum = 1;
    m_lapTimingState = enumReadyToStart;
    m_fxImpulseAcc = 0;
    m_fxImpulseAngle = 0;

    // Reset physics:

    m_fxVel_x = 0;
    m_fxVel_y = 0;
    m_isSliding = false;
    m_isGrounded = true;

    m_steering = 0;
    m_throttle = 0;
    m_braking = 0;

    m_booster = false;
}

void CShip::AIUpdate(TileType tileType)
{
    SetSteering(0);
    SetThrottle(0);
    SetBraking(0);

    // Direction vector to the current waypoint.
    int16_t distX = waypoints[m_activeWaypointIndex].x - fix16_to_int(m_fxX);
    int16_t distY = waypoints[m_activeWaypointIndex].y - fix16_to_int(m_fxY);
    const int16_t radius = fix16_to_int(waypoints[m_activeWaypointIndex].fxRadius);

    // Check if ship is inside the radius of the waypoint and if so, select the next waypoint as target
    if(radius >= distX && radius >= -distX && radius >= distY && radius >= -distY) {
        // Ship is inside the bounding box of waypoint
        const int32_t distSq = distX*distX+distY*distY;

        if(distSq <= radius*radius) {
            // Ship is inside the waypoint radious.

            // Next waypoint.
            ++m_activeWaypointIndex;
            if(m_activeWaypointIndex >= waypointCount) {
                m_activeWaypointIndex = 0;
            }
            // Direction vector to the waypoint.
            distX = waypoints[m_activeWaypointIndex].x - fix16_to_int(m_fxX);
            distY = waypoints[m_activeWaypointIndex].y - fix16_to_int(m_fxY);
        }
    }

    // Calculate vector from target to next waypoint
    int8_t nextWaypointIndex = m_activeWaypointIndex+1;
    if(nextWaypointIndex >= waypointCount) {
        nextWaypointIndex = 0;
    }
    int16_t toNextX = waypoints[nextWaypointIndex].x - waypoints[m_activeWaypointIndex].x;
    int16_t toNextY = waypoints[nextWaypointIndex].y - waypoints[m_activeWaypointIndex].y;

    // Calculate distance needed to eliminate the perpendicular velocity component
    fix16_t fxTargetToNextAngle = fix16_atan2(toNextY, toNextX); // arguments don't really need to be fix16_t
    fix16_t fxPerpAngle = fxTargetToNextAngle+(fix16_pi>>1);
    fix16_t fxPerpSpeed = fix16_mul(fix16_cos(fxPerpAngle), VelocityX())+fix16_mul(fix16_sin(fxPerpAngle), VelocityY());
    fix16_t fxBrakingDist = CalculateBrakingDistance(fxPerpSpeed, tileType);

    // Calculate perpendicular distance to the target waypoint
    fix16_t fxPerpDist = abs(fix16_cos(fxPerpAngle)*distX+fix16_sin(fxPerpAngle)*distY);
    fix16_t fxSpeed = fix16_mul(fix16_cos(Yaw()), VelocityX())+fix16_mul(fix16_sin(Yaw()), VelocityY());

    // If there is no danger of sliding over the route line, then set direction to target and set throttle to full,
    // otherwise prepare for next waypoint by aligning ship direction to the route line between target waypoint and next waypoint
    fix16_t fxTargetAngle = fxTargetToNextAngle;
    if(fxBrakingDist < fxPerpDist) {
        fxTargetAngle = fix16_atan2(distY, distX);
        // Less aggressive AI underestimates the perpendicular distance to the route line
        if(fxBrakingDist < fix16_mul(m_fxCoef_aiAggr, fxPerpDist)) {
            SetThrottle(100);
        }
    }

    // Steer ship towards target angle
    fix16_t fxAngleDiff = fxTargetAngle-Yaw();
    if(fxAngleDiff < -fix16_pi) {
        fxAngleDiff += 2*fix16_pi;
    }
    if(fxAngleDiff > m_fxNpcSteeringDeadzone) {
        SetSteering(100);
    }
    else if(fxAngleDiff < -m_fxNpcSteeringDeadzone) {
        SetSteering(-100);
    }

    // Limit ship speed if waypoint has target speed set to less than 100
    const int16_t targetSpeed = fix16_to_int(waypoints[m_activeWaypointIndex].fxTargetSpeed);
    if(targetSpeed < 100) {
        if(fxSpeed > m_fxCoef_npcNominalSpeed*targetSpeed) {
            SetThrottle(0);
            SetBraking(100);
        }
    }
}

void CShip::StartBooster()
{
    if(!m_boosterAnimValue) {
        m_boosterAnimValue = CAnimValue::GetFreeElement();
        m_boosterAnimValue->Start(1000, fix16_one, fix16_one, this, 1);
        m_booster = true;
    }
}

void CShip::StartJump()
{
    if(!m_jumpAnimValue) {
        m_jumpAnimValue = CAnimValue::GetFreeElement();
        m_jumpAnimValue->Start(1500, 0, fix16_pi, this, 0);
        m_isGrounded = false;
    }
}

void CShip::PhysicsUpdate(TileType tileType)
{
    // Apply drag
    m_fxVel_x -= fix16_mul(abs(m_fxVel_x), fix16_mul(m_fxVel_x, m_fxFactor_drag))/m_mass;
    m_fxVel_y -= fix16_mul(abs(m_fxVel_y), fix16_mul(m_fxVel_y, m_fxFactor_drag))/m_mass;

    // Calculate velocity component parallel to wheel direction
    fix16_t fxDir_x = cos_taylor(m_fxAngle);
    fix16_t fxDir_y = sin_taylor(m_fxAngle);
    fix16_t fxSpeed_w = fix16_mul(fxDir_x, m_fxVel_x)+fix16_mul(fxDir_y, m_fxVel_y);
    int8_t signSpeed_w = 1;
    if(fxSpeed_w < 0) {
        signSpeed_w = -1;
        fxSpeed_w = -fxSpeed_w;
    }

    // Apply steering to ship direction
    if(0 != m_steering) {
        m_fxAngle += m_shipParms.fxCoef_turn*m_steering;
        if((fix16_pi<<1) < m_fxAngle) { m_fxAngle -= (fix16_pi<<1); }
        else if(0 > m_fxAngle) { m_fxAngle += (fix16_pi<<1); }
        fxDir_x = cos_taylor(m_fxAngle);
        fxDir_y = sin_taylor(m_fxAngle);
    }

    m_isSliding = false;
    if(m_isGrounded) {
        PhysicsParameters* physParms = &m_physParmsArray[tileType];

        // Calculate tire velocity change caused by rolling resistance and braking
        fix16_t fxDeltaVel_lon = fix16_div(physParms->fxCoef_rr+m_braking*m_shipParms.fxCoef_brake, physParms->fxSlipFactor);
        if(fxDeltaVel_lon < fxSpeed_w) {
            fxSpeed_w -= fxDeltaVel_lon;
        }
        else {
            fxSpeed_w = 0;
        }

        // Calculate x and y components of forward velocity
        fix16_t fxVel_wx = signSpeed_w*fix16_mul(fxDir_x, fxSpeed_w);
        fix16_t fxVel_wy = signSpeed_w*fix16_mul(fxDir_y, fxSpeed_w);

        // Calculate wheel slip speed
        fix16_t fxSlip_x = fxVel_wx-m_fxVel_x;
        fix16_t fxSlip_y = fxVel_wy-m_fxVel_y;
        fix16_t fxSlip_angle = fix16_atan2(fxSlip_y, fxSlip_x);
        fix16_t fxSlip = fix16_mul(cos_taylor(fxSlip_angle), fxSlip_x)+fix16_mul(sin_taylor(fxSlip_angle), fxSlip_y);

        // Calculate traction from tire slip
        fix16_t fxDeltaVel = fix16_mul(fxSlip, physParms->fxSlipFactor);
        if(fxDeltaVel > physParms->fxDDVel_fs) {
            fxDeltaVel = physParms->fxDDVel_fk;
            m_isSliding = true;
        }
        fxDeltaVel = fix16_mul(fxDeltaVel, m_fxDeltaTime);
        m_fxVel_x += fix16_mul(fxDeltaVel, cos_taylor(fxSlip_angle));
        m_fxVel_y += fix16_mul(fxDeltaVel, sin_taylor(fxSlip_angle));
    }

    // Calculate thrust
    fix16_t fxThrust = fix16_mul(m_throttle*m_shipParms.fxCoef_thrust, m_fxDeltaTime);
    if(m_booster) {
        fxThrust <<= 1;
    }
    m_fxVel_x += fix16_mul(fxThrust, fxDir_x);
    m_fxVel_y += fix16_mul(fxThrust, fxDir_y);
}

void CShip::SetPhysicsParameters(TileType type, fix16_t fxStaticFriction, fix16_t fxKineticFriction, uint16_t tractionConstant, fix16_t fxRollingResistance)
{
    m_physParmsArray[type].fxDDVel_fs = fix16_mul(fxStaticFriction, m_fxGravity);
    m_physParmsArray[type].fxDDVel_fk = fix16_mul(fxKineticFriction, m_fxGravity);
    m_physParmsArray[type].fxSlipFactor = fix16_div(tractionConstant*fix16_one, m_mass*fix16_one+tractionConstant*m_fxDeltaTime);
    m_physParmsArray[type].fxCoef_rr = fix16_mul(fxRollingResistance, m_fxGravity);
}

void CShip::SetShipParameters(fix16_t fxMaxTurn, fix16_t fxMaxThrust, fix16_t fxMaxBrake)
{
    m_shipParms.fxCoef_turn = fxMaxTurn/100;
    m_shipParms.fxCoef_thrust = fxMaxThrust/100;
    m_shipParms.fxCoef_brake = fix16_mul(fxMaxBrake, m_fxGravity)/100;
}

void CShip::ResetDefaultPhysicsParameters()
{
    SetPhysicsParameters(enumTrackTile, 2.0*fix16_one, 1.9*fix16_one, 1500, 0.02*fix16_one);
    SetPhysicsParameters(enumTerrainTile, 0.9*fix16_one, 0.9*fix16_one, 500, 0.2*fix16_one);
    SetPhysicsParameters(enumEdgeTile, 1.5*fix16_one, 1.4*fix16_one, 1500, 0.05*fix16_one);
}

void CShip::ResetDefaultShipParameters()
{
    SetShipParameters(2*fix16_pi/180, 6.0*fix16_one, 0.9*fix16_one);
}

fix16_t CShip::CalculateBrakingDistance(fix16_t fxSpeed, TileType tileType)
{
    // Estimate stopping distance based on brake power or kinetic friction, which ever is weaker
    fix16_t fxDDVel = m_shipParms.fxCoef_brake*100;
    if(fxDDVel > m_physParmsArray[tileType].fxDDVel_fk) {
        fxDDVel = m_physParmsArray[tileType].fxDDVel_fk;
    }
    fxSpeed = ScaleFromGameCoords(fxSpeed);
    return ScaleToGameCoords(fix16_div(fix16_mul(fxSpeed, fxSpeed), fxDDVel)>>1);
}

// Animation finished.
void CShip::Finished(int32_t par)
{
    if(par == 0) {
        m_jumpAnimValue = nullptr;
        m_isGrounded = true;
    }
    else if(par == 1) {
        m_boosterAnimValue = nullptr;
        m_booster = false;
    }
}
