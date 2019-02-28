#include "Pokitto.h"
#include "PokittoCookie.h"
#include "fix16.h"
#include "playerShip.h"

const uint8_t amplitude = 170;
const int8_t arpmode=0;

CPlayerShip::CPlayerShip()
{
    m_final_lap_time_ms = 0;
    m_start_ms = 0;

    // *** Setup sound
    m_tonefreq=0;
    m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;
    m_fxCameraBehindPlayerCurrent = fxCameraBehindPlayerY;
}

void CPlayerShip::Update()
{
    m_bitmap = billboard_object_bitmaps[0];  // org car

    // *** Check collision to road edges
    TileType contactTileType = enumTrackTile;
    uint8_t wavetype = 1;
    uint8_t tileIndex = GetTileIndexCommon(fix16_to_int(m_fxX), fix16_to_int(m_fxY));
    if(tileIndex >= 1 && tileIndex <= 4) {
        contactTileType = enumEdgeTile;
        if(!m_jumpAnimValue) {
            wavetype = 5;
        }
    }
    else if((tileIndex >= 7 && tileIndex <= 10) || tileIndex == 0) {
        contactTileType = enumTerrainTile;
        if(!m_jumpAnimValue) {
            wavetype = 5;
        }
    }
    else if(tileIndex == 15) {
        // Booster.
        CShip::StartBooster();
    }
    else if(tileIndex == 16) {
        // Jump from the ramp.
        CShip::StartJump();
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
    CShip::PhysicsUpdate(contactTileType);
    m_fxX += fix16_mul((fxVel_x0+CShip::VelocityX())>>1, CShip::DeltaTime());
    m_fxY += fix16_mul((fxVel_y0+CShip::VelocityY())>>1, CShip::DeltaTime());
    // ======================================

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

    // Update sound effect
    if(CShip::IsBoosterActive()) {
        // Booster sound
        setOSC(&osc1,1,/*wavetype*/4,1,0,0,/*m_tonefreq*/40,amplitude,0,0,0,0,0,0,arpmode,1,0);
    }
    else if (IsSliding()) {
        setOSC(&osc1,1,/*wavetype*/2,1,0,0,/*m_tonefreq*/40,amplitude,0,0,0,0,0,0,arpmode,1,0);
    }
    else {
        // Sound by the speed
        fix16_t fxVel = fix16_mul(fix16_cos(CShip::Yaw()), CShip::VelocityX())+fix16_mul(fix16_sin(CShip::Yaw()), CShip::VelocityY());
        int16_t speed = abs(fix16_to_int(fxVel));
        m_tonefreq = 70*speed/(210+speed);
        if(m_tonefreq > 50) {
            m_tonefreq = 50;
        }
        setOSC(&osc1,1,wavetype,1,0,0,m_tonefreq,amplitude,0,0,0,0,0,0,arpmode,1,0);
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
            if( g_isRace) {
                CalculateRank();
            }
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
            m_activeWaypointIndex = -1;
            CalculateRank();
        }
        break;

    case enumFinished:
        break;
    }
}

void CPlayerShip::FindActiveWaypoint()
{

    // *** Update visited waypoints

    // Check the 5 next waypoints, one at a frame.
    m_lastCheckedWPIndex++;
    if( ++m_lastCheckedWPIndex >= waypointCount + 5)
        m_lastCheckedWPIndex -= waypointCount;  // new round
    if( m_lastCheckedWPIndex > m_activeWaypointIndex + 5 || m_lastCheckedWPIndex < m_activeWaypointIndex )
       m_lastCheckedWPIndex = m_activeWaypointIndex + 1;

    // Calc real index.  m_lastCheckedWPIndex can be bigger than waypointCount.
    int8_t lastCheckedWPIndex = m_lastCheckedWPIndex;
    if(lastCheckedWPIndex >= waypointCount ) {
        lastCheckedWPIndex -= waypointCount;
    }

    // Direction vector to the current waypoint.
    int16_t distX = waypoints[lastCheckedWPIndex].x - fix16_to_int(m_fxX);
    int16_t distY = waypoints[lastCheckedWPIndex].y - fix16_to_int(m_fxY);
    const int16_t radius = fix16_to_int(waypoints[lastCheckedWPIndex].fxRadius);

    // Check ship's distance to the waypoint
    if(radius >= distX && radius >= -distX && radius >= distY && radius >= -distY) {
        // Ship is inside the bounding box of the waypoint

        const int32_t distSq = distX*distX+distY*distY;
        if(distSq <= radius*radius) {
            // Ship is inside the waypoint radious.

            if(m_activeWaypointIndex != lastCheckedWPIndex) {
                m_activeWaypointIndex = lastCheckedWPIndex;
                m_lastCheckedWPIndex = -1; // None checked
                m_activeWaypointFoundTimeInMs = mygame.getTime();

                CalculateRank();
            }
        }
    }
}

void CPlayerShip::CalculateRank()
{
    int32_t numOfCarsWithBetterRank = 0;
    int8_t activeWaypointIndex = m_activeWaypointIndex + 1;  // active waypoint is 1 less for player than for other ships
    for(int32_t i=1; i < g_shipCount; i++)
    {
        if(g_ships[i]->m_activeLapNum > m_activeLapNum) {
            numOfCarsWithBetterRank++;
        }
        else if(g_ships[i]->m_activeLapNum == m_activeLapNum) {
            int8_t wpIndex = (g_ships[i]->m_activeWaypointIndex == 0) ? waypointCount : g_ships[i]->m_activeWaypointIndex;
            if(wpIndex >= activeWaypointIndex) {
                numOfCarsWithBetterRank++;
            }
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
    m_fxAngle = fix16_pi>>1;
    m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;
    m_fxCameraBehindPlayerCurrent = fxCameraBehindPlayerY;
    m_currentRank = 0;
    m_current_lap_time_ms = 0;
    m_requestedMenuMode = CMenu::enumNoMenu;
    //!!!HV snd.ampEnable(1);
    //snd.playTone(1,m_tonefreq,amplitude,wavetype,arpmode);
    m_tonefreq = 0;
    setOSC(&osc1,1,/*wavetype*/1,1,0,0,m_tonefreq,/*amplitude*/0,0,0,0,0,0,0,arpmode,1,0);
    //for( int32_t i = 0; i < 50 && i < waypointCount; i++ ) m_waypointsVisited[ i ] = false;
    m_activeWaypointIndex = -1;
    m_lastCheckedWPIndex = -1;
    m_activeWaypointFoundTimeInMs = 0;
 }

// Handle keys
void CPlayerShip::HandleGameKeys()
{

#if 1
    // Playing

    // Debug: calculate available RAM is all button , A, B and C, are pressed!
    if( mygame.buttons.aBtn() && mygame.buttons.bBtn() && mygame.buttons.cBtn() )
        CalcFreeRamAndHang();

    // Turn left
    if(mygame.buttons.leftBtn()) {
        CShip::SetSteering(100);
    }
    // Turn right
    else if(mygame.buttons.rightBtn()) {
        CShip::SetSteering(-100);
    }
    else {
        CShip::SetSteering(0);
    }

    CShip::SetThrottle(0);
    CShip::SetBraking(0);
    m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY;

    // Thrust
    if(mygame.buttons.aBtn()) {
        CShip::SetThrottle(100);
        m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY + fix16_from_int(3);
    }

    // Reverse
    if(mygame.buttons.bBtn()) {
        CShip::SetBraking(100);
        m_fxCameraBehindPlayerTarget = fxCameraBehindPlayerY - fix16_from_int(5);
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
