#include "Pokitto.h"
#include "PokittoCookie.h"
#include "fix16.h"
#include "playerShip.h"

const uint8_t amplitude = 170;
const int8_t wavetype = 1;
const int8_t wavetypeCrash = 4;
const int8_t arpmode=0;
const fix16_t fxRotAccFactor = fix16_from_float(1.2);

CPlayerShip::CPlayerShip()
{
    m_final_lap_time_ms = 0;
    m_start_ms = 0;
    m_isCollided = false;
    m_isTurningLeft = false;
    m_isTurningRight = false;
    m_jumpAnimValue = NULL;
    m_boosterAnimValue = NULL;

    // *** Setup sound
    m_tonefreq=0;
    m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;
    m_fxCameraBehindPlayerCurrent = fxCameraBehindPlayerY;
}

void CPlayerShip::Update()
{
    fix16_t fxVelOld = fix16_mul(fix16_cos(Yaw()), VelocityX())+fix16_mul(fix16_sin(Yaw()), VelocityY());
    fxVelOld /= 35; // Scale to make it compatible with pre-physics stuff
    bool prevCollided = m_isCollided;

    m_bitmap = billboard_object_bitmaps[0];  // org car

    // Calcucalate the current waypoint and rank.
    if( m_doRecalcRank )
        CalculateRank();
    m_doRecalcRank = false;

    // *** Check collision to road edges
    m_isCollided = false;
    uint8_t wavetype = 1;
    //uint8_t tileIndex = 5;
    uint8_t tileIndex = GetTileIndexCommon(fix16_to_int(m_fxX), fix16_to_int(m_fxY));
    if( (tileIndex == 16 ) )
    {
        // Jump from the ramp.
        if( ! m_jumpAnimValue )
        {
            m_jumpAnimValue = CAnimValue::GetFreeElement();
            m_jumpAnimValue->Start( 1500, 0, fix16_pi, this, 0 );
            CShip::ToggleGrounded(false);
        }
    }
    else if( tileIndex == 15 )
    {
        // Booster.
        if( ! m_boosterAnimValue )
        {
            m_boosterAnimValue = CAnimValue::GetFreeElement();
            m_boosterAnimValue->Start( 1000, fix16_one, fix16_one, this, 1 );
            CShip::ToggleBooster(true);
        }
    }
    else if( ! m_jumpAnimValue && tileIndex != 5 && tileIndex != 6 &&
        (tileIndex < 11 || tileIndex > 15) )
    {
        // Collided to the terrain.
        m_isCollided = true;
        wavetype = 5;
    }

    TileType tileType = enumTrackTile;
    if(tileIndex >= 1 && tileIndex <= 4) {
        tileType = enumEdgeTile;
    }
    else if(tileIndex >= 7 && tileIndex <= 10) {
        tileType = enumTerrainTile;
    }

    // get the current lap time
    if( m_lapTimingState == enumReadyToStart )
        m_current_lap_time_ms = 0;
    else if( m_lapTimingState == enumFinished )
        m_current_lap_time_ms = m_final_lap_time_ms;
    else
        m_current_lap_time_ms = mygame.getTime() - m_start_ms;

    // Handle race/time trial starting and ending detection.
    bool isOnStartingGrid = ( tileIndex >= 11 && tileIndex <= 14);
    HandleRaceStartingAndEnding( isOnStartingGrid );

    // Find active waypoint
    FindActiveWaypoint();

    // Read keys.
    HandleGameKeys();

    // ======================================
    // Physics based driving model experiment
    fix16_t fxVel_x0 = CShip::VelocityX();
    fix16_t fxVel_y0 = CShip::VelocityY();
    CShip::PhysicsUpdate(tileType);
    m_fxX += fix16_mul((fxVel_x0+CShip::VelocityX())>>1, CShip::DeltaTime());
    m_fxY += fix16_mul((fxVel_y0+CShip::VelocityY())>>1, CShip::DeltaTime());
    m_fxVel = fix16_mul(fix16_cos(CShip::Yaw()), CShip::VelocityX())+fix16_mul(fix16_sin(CShip::Yaw()), CShip::VelocityY());
    m_fxVel /= 35; // Scale to make it compatible with pre-physics stuff
    // ======================================

    // Boost
    if( m_boosterAnimValue )
        m_fxVel = g_fxBoostVel;

    // If colliding, slow down
    if( m_isCollided ) {

        // Break or stop
        if(m_fxVel > fxMaxSpeedCollided)
        {
            m_fxVel -= (fix16_one>>4);
            if(m_fxVel < 0)
                m_fxVel = 0;
        }
        else if(m_fxVel < -fxMaxSpeedCollided)
        {
            m_fxVel += (fix16_one>>4);
            if(m_fxVel > 0)
                m_fxVel = 0;
        }
    }

    // Limit speed
    if(m_fxVel>fxMaxSpeed && m_boosterAnimValue==NULL )
        m_fxVel = fxMaxSpeed;
    else if(m_fxVel<-fxMaxSpeed)
        m_fxVel = -fxMaxSpeed;

    if( m_fxImpulseAcc != 0 )
    {
        // Move ship along the impulse.
        fix16_t fxImpulseCos = fix16_cos(m_fxImpulseAngle);
        fix16_t fxImpulseSin = fix16_sin(m_fxImpulseAngle);
        m_fxX += fix16_mul(m_fxImpulseAcc, fxImpulseCos);
        m_fxY += fix16_mul(m_fxImpulseAcc, fxImpulseSin);

        // Reset impulse
        m_fxImpulseAngle = 0;
        m_fxImpulseAcc = 0;
    }

//    m_fxX += fxVelX;
//    m_fxY += fxVelY;

    // Change sound effect if needed.
    if(fxVelOld != m_fxVel || prevCollided != m_isCollided )
    {
        if(m_boosterAnimValue)
        {
            // Booster sound
            setOSC(&osc1,1,/*wavetype*/4,1,0,0,/* m_tonefreq */ 40,amplitude,0,0,0,0,0,0,arpmode,1,0);
        }
        else
        {
            // Sound by the speed
            m_tonefreq = (IsSliding()) ? 35 : fix16_to_int(abs(m_fxVel*5));
            if(m_tonefreq>50) m_tonefreq = 50;
                //snd.playTone(1,m_tonefreq,amplitude,wavetype,arpmode);
            setOSC(&osc1,1,wavetype,1,0,0,m_tonefreq,amplitude,0,0,0,0,0,0,arpmode,1,0);
        }
    }

    // Update camera pos

    //fix16_t fxDiff =  m_fxCameraBehindPlayerTarget - m_fxCameraBehindPlayerCurrent;
    //fxDiff >>= 1;
    //m_fxCameraBehindPlayerCurrent += fxDiff;

    if(m_fxCameraBehindPlayerTarget > m_fxCameraBehindPlayerCurrent)
    {
        m_fxCameraBehindPlayerCurrent += fix16_one>>2;
    }
    else if(m_fxCameraBehindPlayerTarget < m_fxCameraBehindPlayerCurrent)
    {
        m_fxCameraBehindPlayerCurrent -= fix16_one>>2;
    }
}

void CPlayerShip::HandleRaceStartingAndEnding( bool isOnStartingGrid )
{
    switch(m_lapTimingState)
    {
    case enumReadyToStart:
        if( isOnStartingGrid )
        {
            m_lapTimingState = enumStarted;
            m_start_ms = mygame.getTime();  // started
        }
        break;
    case enumStarted:
        if( ! isOnStartingGrid )
        {
            m_lapTimingState = enumOnTimedTrack;
        }
        break;
    case enumOnTimedTrack:
        if( isOnStartingGrid && m_activeWaypointIndex > waypointCount - 5 )  // At least the one of the 5 last waypoints visited
        {
            // Finished!
            m_final_lap_time_ms = mygame.getTime() - m_start_ms;

            // Open the menu after the race.
            if( g_isRace)
            //if( g_isRace )
            {
                if( m_activeLapNum>2)
                {
                    // Race finished
                    m_requestedMenuMode = CMenu::enumRaceFinishedMenu;
                    m_lapTimingState = enumFinished;

                    // Play "Finished" song.
                    SetupMusic(2);
                }
                else
                {
                    m_lapTimingState = enumStarted;
                }
            }
            else
            {
                // Play "Finished" song.
                SetupMusic(2);

                // Open the menu after the time trial.
                m_requestedMenuMode = CMenu::enumTimeTrialFinishedMenu;
            }

            // Save cookie if this is the best time
            SaveHighScore(m_final_lap_time_ms);

            m_activeLapNum++;
        }
        break;

    case enumFinished:
        break;
    }
}

void CPlayerShip::FindActiveWaypoint()
{

    // *** Update visited waypoints

    // Direction vector to the current waypoint. Check the 5 next waypoints, one at a frame.
    m_lastCheckedWPIndex++;
    if( ++m_lastCheckedWPIndex >= waypointCount + 5)
        m_lastCheckedWPIndex -= waypointCount;  // new round
    if( m_lastCheckedWPIndex > m_activeWaypointIndex + 5 || m_lastCheckedWPIndex < m_activeWaypointIndex )
       m_lastCheckedWPIndex = m_activeWaypointIndex + 1;

    // Calc real index.  m_lastCheckedWPIndex can be bigger than waypointCount.
    int8_t lastCheckedWPIndex = m_lastCheckedWPIndex;
    if(lastCheckedWPIndex >= waypointCount )
        lastCheckedWPIndex -= waypointCount;


    fix16_t fxDirX = fix16_from_int(waypoints[ lastCheckedWPIndex].x ) - m_fxX;
    fix16_t fxDirY = fix16_from_int(waypoints[ lastCheckedWPIndex].y ) - m_fxY;

    // Calculate distance.
    // Scale down so that it will not overflow
    fix16_t fxX3 = fxDirX;
    fix16_t fxY3 = fxDirY;
    fxX3 >>= 4;
    fxY3 >>= 4;
    fix16_t fxDistanceToWaypoint = fix16_mul(fxX3, fxX3) + fix16_mul(fxY3,fxY3);
    const fix16_t fxLimit = fix16_from_int(20*20);
    if( fxDistanceToWaypoint < fxLimit>>4 )
    {
        m_activeWaypointIndex = lastCheckedWPIndex;
        m_lastCheckedWPIndex = -1; // None checked
        m_activeWaypointFoundTimeInMs = mygame.getTime();
        m_doRecalcRank = true;
        //m_waypointsVisited[ m_lastVisitedWPIndex ] = true;
    }
}

void CPlayerShip::CalculateRank()
{
    int32_t numOfCarsWithBetterRank = 0;
    int8_t activeWaypointIndex = m_activeWaypointIndex + 1;  // active waypoint is 1 less for player than for other ships
    for(int32_t i=1; i < g_shipCount; i++)
    {
        if( g_ships[i]->m_activeLapNum > m_activeLapNum ||
            ( g_ships[i]->m_activeLapNum == m_activeLapNum && g_ships[i]->m_activeWaypointIndex >= activeWaypointIndex ))
        {
            numOfCarsWithBetterRank++;
        }
    }

    m_currentRank = numOfCarsWithBetterRank+1;
}

void CPlayerShip::Reset()
{
    CShip::Reset();

     // Reset game
    m_fxX = fix16_from_int(45);
    m_fxY = fix16_from_int(550);
    m_fxVel = 0;
    m_fxAngle = fix16_pi>>1;
    m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;
    m_fxCameraBehindPlayerCurrent = fxCameraBehindPlayerY;
    m_currentRank = 0;
    m_current_lap_time_ms = 0;
    m_requestedMenuMode = CMenu::enumNoMenu;
    //!!!HV snd.ampEnable(1);
    //snd.playTone(1,m_tonefreq,amplitude,wavetype,arpmode);
    m_tonefreq = 0;
    setOSC(&osc1,1,wavetype,1,0,0,m_tonefreq,amplitude,0,0,0,0,0,0,arpmode,1,0);
    //for( int32_t i = 0; i < 50 && i < waypointCount; i++ ) m_waypointsVisited[ i ] = false;
    m_activeWaypointIndex = 0;
    m_lastCheckedWPIndex = -1;
    m_doRecalcRank = false;
    m_activeWaypointFoundTimeInMs = 0;
    if( m_jumpAnimValue )
        m_jumpAnimValue->Reset();
    m_jumpAnimValue = NULL;
    if( m_boosterAnimValue )
        m_boosterAnimValue->Reset();
    m_boosterAnimValue = NULL;
 }

// Handle keys
void CPlayerShip::HandleGameKeys()
{

#if 1
    // Playing

    // Debug: calculate available RAM is all button , A, B and C, are pressed!
    if( mygame.buttons.aBtn() && mygame.buttons.bBtn() && mygame.buttons.cBtn() )
        CalcFreeRamAndHang();

    CShip::SetSteering(0);
    CShip::SetThrottle(0);
    CShip::SetBraking(0);

    // Turn left
    if(mygame.buttons.leftBtn()) {
        CShip::SetSteering(100);

//        if( ! m_isTurningLeft )
//            m_fxRotVel = fxInitialRotVel; // Reset to initial velocity when started turning
//        m_fxAngle += m_fxRotVel;
//        m_isTurningLeft = true;
//        m_fxRotVel = fix16_mul(m_fxRotVel, fxRotAccFactor);
    }
//    else {
//        if( m_isTurningLeft )
//            m_fxRotVel = fxInitialRotVel;
//        m_isTurningLeft = false;
//    }

    // Turn right
    if(mygame.buttons.rightBtn()) {
        CShip::SetSteering(-100);

//        if( ! m_isTurningRight )
//            m_fxRotVel = fxInitialRotVel; // Reset to initial velocity when started turning
//        m_fxAngle -= m_fxRotVel;
//        m_isTurningRight = true;
//        m_fxRotVel = fix16_mul(m_fxRotVel, fxRotAccFactor);
    }
//    else {
//        if( m_isTurningRight )
//            m_fxRotVel = fxInitialRotVel;
//        m_isTurningRight = false;
//    }


    // Thrust
    if(mygame.buttons.aBtn()) {
        CShip::SetThrottle(100);

        if(!m_isCollided || m_fxVel<=fxMaxSpeedCollided)
        {
//            m_fxVel = m_fxVel + (fix16_one>>4);
            m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY + fix16_from_int(3);
        }
    }

    // Reverse
    else if(mygame.buttons.bBtn()) {
        CShip::SetBraking(100);

        if(!m_isCollided || m_fxVel>=fxMaxSpeedCollided)
        {
//            m_fxVel = m_fxVel - (fix16_one>>4);
            m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY - fix16_from_int(5);
        }
    }
    // Break a little (friction) if no A button is pressed
    else  {

        if(!m_isCollided || m_fxVel>=fxMaxSpeedCollided)
        {
            m_fxVel = m_fxVel - (fix16_one>>5); // Friction
            m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;
        }

        if(m_fxVel < 0)
            m_fxVel = 0;
    }

#else
    if(mygame.buttons.leftBtn()) {
        m_fxX += fix16_one;
    }
    else if(mygame.buttons.rightBtn()) {
        m_fxX -= fix16_one;
    }
    else if(mygame.buttons.upBtn()) {
        m_fxY += fix16_one;
    }
    else if(mygame.buttons.downBtn()) {
        m_fxY -= fix16_one;
    }
    else if(mygame.buttons.aBtn()) {
        m_fxAngle += fxRotVel;
    }
    else if(mygame.buttons.bBtn()) {
        m_fxAngle -= fxRotVel;
    }

#endif
}

// Animation finished.
void CPlayerShip::Finished( int32_t par )
{
    if(par==0) {
        m_jumpAnimValue = NULL;
        CShip::ToggleGrounded(true);
    }
    else if(par==1) {
        m_boosterAnimValue = NULL;
        CShip::ToggleBooster(false);
    }
}

