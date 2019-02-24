#include "Pokitto.h"
#include "PokittoCookie.h"
#include "fix16.h"
#include "ship.h"

CShip::PhysicsParameters CShip::m_physParmsArray[3] = {
    {
        (fix16_t)(fix16_one*1.5*9.81*1.0/30),
        (fix16_t)(fix16_one*1.2*9.81*1.0/30),
        (fix16_t)(fix16_one*2000*1.0/30/(m_mass+2000*1.0/30)),
        (fix16_t)(fix16_one*0.020)
    }, {
        (fix16_t)(fix16_one*1.3*9.81*1.0/30),
        (fix16_t)(fix16_one*1.0*9.81*1.0/30),
        (fix16_t)(fix16_one*2000*1.0/30/(m_mass+2000*1.0/30)),
        (fix16_t)(fix16_one*0.030)
    }, {
        (fix16_t)(fix16_one*0.6*9.81*1.0/30),
        (fix16_t)(fix16_one*0.6*9.81*1.0/30),
        (fix16_t)(fix16_one*1000*1.0/30/(m_mass+1000*1.0/30)),
        (fix16_t)(fix16_one*0.040)
    }
};

CShip::ShipParameters CShip::m_shipParms = {
    (fix16_t)(2*fix16_pi/180/100), // turn coefficient
    (fix16_t)(3.5*fix16_one/100),  // thrust coefficient
    (fix16_t)(0.9*fix16_one/100)   // brake coefficient
};


CShip::CShip() :
    m_fxImpulseAcc(0),
    m_fxImpulseAngle(0),
    m_fxAngle(0),
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

void CShip::Reset()
{
    m_fxImpulseAcc = 0;
    m_fxImpulseAngle = 0;

    m_fxAngle = 0;
    m_fxVel_x = 0;
    m_fxVel_y = 0;
    m_isSliding = false;
    m_isGrounded = true;

    m_steering = 0;
    m_throttle = 0;
    m_braking = 0;

    m_booster = false;
}

void CShip::SetPhysicsParameters(TileType type, fix16_t fxStaticFriction, fix16_t fxKineticFriction, uint16_t tractionConstant, fix16_t fxRollingResistance)
{
    fix16_t fxTemp = fix16_mul(m_fxGravity, m_fxDeltaTime);
    m_physParmsArray[type].fxDeltaVel_fs = fix16_mul(fxStaticFriction, fxTemp);
    m_physParmsArray[type].fxDeltaVel_fk = fix16_mul(fxKineticFriction, fxTemp);
    m_physParmsArray[type].fxSlipFactor = fix16_div(tractionConstant*m_fxDeltaTime, m_mass*fix16_one+tractionConstant*m_fxDeltaTime);
    m_physParmsArray[type].fxCoef_rr = fxRollingResistance;
}

void CShip::SetShipParameters(fix16_t fxMaxTurn, fix16_t fxMaxThrust, fix16_t fxMaxBrake)
{
    m_shipParms.fxCoef_turn = fxMaxTurn/100;
    m_shipParms.fxCoef_thrust = fxMaxThrust/100;
    m_shipParms.fxCoef_brake = fxMaxBrake/100;
}

void CShip::UpdatePhysics()
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
        // Check what type of tile the wheel is on
        PhysicsParameters* physParms = &m_physParmsArray[enumTrackTile];
        uint8_t tileIndex = GetTileIndexCommon(fix16_to_int(m_fxX), fix16_to_int(m_fxY));
        if(tileIndex >= 1 && tileIndex <= 4) {
            // Edge
            physParms = &m_physParmsArray[enumEdgeTile];
        }
        else if(tileIndex >= 7 && tileIndex <= 10) {
            // Terrain
            physParms = &m_physParmsArray[enumTerrainTile];
        }

        // Calculate tire velocity change caused by rolling resistance and braking
        fix16_t fxTemp = fix16_div(fix16_mul(m_fxGravity, m_fxDeltaTime), physParms->fxSlipFactor);
        fix16_t fxDeltaVel_lon = fix16_mul((physParms->fxCoef_rr+m_braking*m_shipParms.fxCoef_brake), fxTemp);
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
        if(fxDeltaVel > physParms->fxDeltaVel_fs) {
            fxDeltaVel = physParms->fxDeltaVel_fk;
            m_isSliding = true;
        }
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

void CShip::SetImpulse(fix16_t fxImpulseAngle)
{
    m_fxImpulseAcc = fix16_one<<2;
    m_fxImpulseAngle = fxImpulseAngle; // -(fix16_pi>>2);
}
