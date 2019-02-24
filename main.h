
#pragma once

#include "fix16.h"
#include "PokittoCookie.h"
#include "animation.h"

#define PGP_VERSION_STRING "1.1.1"

// #define TEST_SHOW_INFO_OF_SHIP_NUM 1

class CShip;
class CPlayerShip;

// Constants
const int32_t screenW = 110;
const int32_t screenH = 88;
const uint8_t mapWidth = 32;
const uint8_t mapHeight = 32;

const uint8_t texW = 8;
const uint8_t texH = 12;
const uint8_t tileW = 8;
const uint8_t tileH = 8;

const uint16_t sceneryH = 16;
const uint16_t screenShiftY = 5;

const int32_t g_rotatingCenterX = -6;
const int32_t g_rotatingCenterY = 72;

const fix16_t fxMaxSpeedCollided = fix16_one>>1;
const fix16_t fxMaxSpeed = fix16_one*6;  // player max speed

const fix16_t fxDefaultOtherShipSpeed = fix16_one*10;
const fix16_t fxDefaultOtherShipSpeedInSlowCorner =  fxMaxSpeedCollided * 7;
const fix16_t fxDefaultOtherShipSpeedInCorner =  fxMaxSpeedCollided * 12;
const fix16_t g_fxBoostVel = fix16_one*10;
//const fix16_t fxDefaultOtherShipSpeed = (fix16_one*10)>>2;
//const fix16_t fxDefaultOtherShipSpeedInCorner =  (fxMaxSpeedCollided * 12)>>2;
//const fix16_t fxDefaultOtherShipSpeedInSlowCorner =  (fxMaxSpeedCollided * 7)>>2;

const fix16_t fxCameraBehindPlayerY = fix16_from_int(70);
//const fix16_t fxDefaultOtherShipSpeed = fix16_one;

//const fix16_t fxMaxSpeedCollided = fix16_one>>4;

const uint32_t g_BillboardObjectArrayMaxCount = 3*8;
const int32_t g_animValueArrayCount = 10;

extern uint32_t g_currentFrameTimeInMs;
extern CAnimValue g_animValueArray[g_animValueArrayCount];

enum LapTimingState {
    enumReadyToStart = 0,
    enumStarted = 1,
    enumOnTimedTrack = 2,
    enumFinished = 4,

};

//
struct Object3dInitData
{
public:
    fix16_t m_fxX;
    fix16_t m_fxY;
    fix16_t m_fxScaledWidth;
    fix16_t m_fxScaledHeight;
    const uint8_t* m_bitmap;
    int16_t m_bitmapW;
    int16_t m_bitmapH;
};

struct Object3dInitDataRom
{
public:
    int8_t m_id;
    int16_t m_x;
    int16_t m_y;
};

class CObject3d
{
public:
    virtual void SetImpulse( fix16_t fxImpulseAngle ) {}

public:
    fix16_t m_fxX;
    fix16_t m_fxY;
    fix16_t m_fxScaledWidth;
    fix16_t m_fxScaledHeight;
    const uint8_t* m_bitmap;
    int16_t m_bitmapW;
    int16_t m_bitmapH;

    fix16_t m_fxXInView;
    fix16_t m_fxYInView;
    fix16_t m_fxDistancePot;
};


class CWaypoint
{
 public:
    int32_t x;
    int32_t y;
    fix16_t fxRadius;
    fix16_t fxTargetSpeed;
    bool isCheckPoint;
};

class mycookie : public Pokitto::Cookie
{
public:
    uint32_t bestLap_ms = 0;
    uint32_t version = 1;
};

// extern
extern Pokitto::Core mygame;
extern Pokitto::Sound snd;
extern fix16_t PerspectiveScaleY[];
extern fix16_t PerspectiveScaleX[];
extern uint8_t* blockMapRAM;
//extern const uint8_t blockMapROM[];
extern uint8_t* blockMap;
extern const uint8_t blockData[][8*8];
extern const uint8_t* default_rom_bitmap_data[];
extern const uint8_t* current_texture_bitmap_data[];
extern const uint8_t* current_texture_bitmap_data_mm1[];
extern const uint8_t* current_texture_bitmap_data_mm2[];
extern uint8_t* g_spriteBitmaps[];
extern CWaypoint waypoints[];
//extern const CWaypoint waypointsROM[];
extern uint32_t waypointCount;
//extern const uint32_t waypointCountROM;
extern const uint32_t waypointMaxCount;
extern const CObject3d g_timeTrialBilboardObjects[];
extern CPlayerShip g_playerShip;
extern const uint8_t image_numbers[];
extern uint16_t g_gamePalette[256];
extern const uint16_t palette_pal[256];
extern int32_t g_billboardObjectInRamCount;
extern CObject3d g_BillboardObjectArray[g_BillboardObjectArrayMaxCount];
extern const uint8_t billboard_object_bitmaps_count;
extern const uint8_t* billboard_object_bitmaps [];
extern uint8_t* g_BackgroundTileBitmap;

extern uint32_t g_frameNum;
extern bool g_isRace;
extern uint32_t g_shipCount;
extern CShip* g_ships[];

// *** Function protptypes

//
uint8_t GetTileIndexCommon(int32_t posX, int32_t posY);
void ResetGame(bool isRace_);
void SetupMusic(int32_t songNumber);

//
void DrawMode7(int32_t tile2PosX, int32_t tile2PosY, fix16_t fxAngle, fix16_t fxRotateCenterX, fix16_t fxRotateCenterY, fix16_t* perspectiveScaleXArr, fix16_t* perspectiveScaleYArr, uint16_t sceneryH2);
void DrawLapTime(int32_t milliseconds, uint32_t activeWpFoundAtInMs, uint32_t x, uint32_t y, fix16_t fxScaleFactor);
void DrawBitmapOpaque8bit(int32_t posX, int32_t posY, const uint8_t* bitmapPtr, uint32_t bitmapW, uint32_t bitmapH );
void DrawTiledBitmapOpaque256ColorPOT(int32_t posX, int32_t posY, const uint8_t* bitmapPtr, const uint32_t bitmapW_POT, uint32_t bitmapH, uint32_t targetW );
void DrawBitmap8bit(int32_t posX, int32_t posY, const uint8_t* bitmapPtr, uint32_t bitmapW, uint32_t bitmapH );
void DrawScaledBitmap8bit(int32_t posX, int32_t posY, const uint8_t* bitmapPtr, uint32_t bitmapW, uint32_t bitmapH, uint32_t scaledW, uint32_t scaledH );
void DrawRankNumber(int32_t x, int32_t y);
void SaveHighScore(uint32_t final_lap_time_ms);
void RestoreRomTextures();
void CalcFreeRamAndHang();
