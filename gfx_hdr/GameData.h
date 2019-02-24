/**************************************************************************/
/*!
    @file     GameData.h
    @author   Hannu Viitala

*/
/**************************************************************************/

#include "Pokitto.h"
#include "fix16.h"
#include "main.h"

#include "palette.h"
#include "image_shadow.h"
#include "image_ship_1_a.h"
#include "image_ship_1_b.h"
#include "image_ship_1_c.h"
#include "image_ship_1_d.h"
#include "image_ship_1_e.h"
#include "image_ship_2_a.h"
#include "image_ship_2_b.h"
#include "image_ship_2_c.h"
#include "image_ship_2_d.h"
#include "image_ship_2_e.h"
#include "image_ship_3_a.h"
#include "image_ship_3_b.h"
#include "image_ship_3_c.h"
#include "image_ship_3_d.h"
#include "image_ship_3_e.h"
#include "image_ship_4_a.h"
#include "image_ship_4_b.h"
#include "image_ship_4_c.h"
#include "image_ship_4_d.h"
#include "image_ship_4_e.h"
#include "image_ship_5_a.h"
#include "image_ship_5_b.h"
#include "image_ship_5_c.h"
#include "image_ship_5_d.h"
#include "image_ship_5_e.h"
#include "image_cactus.h"
#include "image_rock1.h"
#include "image_ball1_a.h"
#include "image_ball1_b.h"
#include "image_ball1_c.h"
#include "image_ball1_d.h"
#include "image_terrain_6_a.h"
#include "image_terrain_6_b.h"
#include "image_terrain_6_c.h"
#include "image_terrain_6_d.h"
#include "image_start_a.h"
#include "image_start_b.h"
#include "image_start_c.h"
#include "image_start_d.h"
#include "image_road1.h"
#include "image_road2.h"
#include "image_sky.h"
#include "image_sky_long.h"
#include "image_light.h"
#include "image_spot.h"
#include "image_boost1.h"
#include "image_ramp1.h"

// Palette
// UI colors ==> 0-7 colors
// image_numbers.bmp ==> 0-10 colors
// image_numbers.bmp image_shadow.bmp image_ship.bmp ==> 0-31 colors
// image_numbers.bmp image_shadow.bmp image_ship.bmp image_sky.bmp ==> 0-40 colors
// all: 0-83 colors

 // Defines blocks of 64 (8x8) tiles
 const uint8_t blockData[][8*8] = {

    // block: 0
    {
        // 8x8 tile indexes.Left edge of the road
        7, 8, 7, 1, 2, 5, 5, 5,
        9,10, 9, 3, 4, 5, 5, 5,
        7, 8, 7, 1, 2, 6, 6, 6,
        9,10, 9, 3, 4, 6, 6, 6,
        7, 8, 7, 1, 2, 5, 5, 5,
        9,10, 9, 3, 4, 5, 5, 5,
        7, 8, 7, 1, 2, 6, 6, 6,
        9,10, 9, 3, 4, 6, 6, 6,
     },
    // block: 1
    {
        // 8x8 tile indexes. Right edge of the road
        5, 5, 5, 1, 2, 8, 7, 8,
        5, 5, 5, 3, 4, 9,10,10,
        6, 6, 6, 1, 2, 8, 7, 8,
        6, 6, 6, 3, 4, 9,10,10,
        5, 5, 5, 1, 2, 8, 7, 8,
        5, 5, 5, 3, 4, 9,10,10,
        6, 6, 6, 1, 2, 8, 7, 8,
        6, 6, 6, 3, 4, 9,10,10,
    },
    // block: 2. Terrain
    {
        // 8x8 tile indexes
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    },
    // block: 3. Road middle block, vertical
    {
        // 8x8 tile indexes
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
    },
     // block: 4
    {
        // 8x8 tile indexes.Top edge of the road
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        1, 2, 1, 2, 1, 2, 1, 2,
        3, 4, 3, 4, 3, 4, 3, 4,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
     },
     // block: 5
    {
        // 8x8 tile indexes. Bottom edge of the road
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        1, 2, 1, 2, 1, 2, 1, 2,
        3, 4, 3, 4, 3, 4, 3, 4,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
    },
    // block: 6. TL outer corner.
    {
        // 8x8 tile indexes.
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 1, 2,
        9,10, 9,10, 9,10, 3, 4,
        7, 8, 7, 8, 1, 2, 6, 6,
        9,10, 9,10, 3, 4, 6, 6,
     },
    // block: 7. TL inner corner.
    {
        // 8x8 tile indexes.
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 1, 2,
        6, 6, 6, 6, 6, 6, 3, 4,
        6, 6, 6, 6, 1, 2, 7, 8,
        6, 6, 6, 6, 3, 4, 9,10,
     },
    // block: 8. BL outer corner.
    {
        // 8x8 tile indexes.
        7, 8, 7, 8, 1, 2, 6, 6,
        9,10, 9,10, 3, 4, 6, 6,
        7, 8, 7, 8, 7, 8, 1, 2,
        9,10, 9,10, 9,10, 3, 4,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
     },
    // block: 9. BL inner corner.
    {
        // 8x8 tile indexes.
        6, 6, 6, 6, 1, 2, 7, 8,
        6, 6, 6, 6, 3, 4, 9,10,
        6, 6, 6, 6, 6, 6, 1, 2,
        6, 6, 6, 6, 6, 6, 3, 4,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
     },
    // block: 10. TR outer corner.
    {
        // 8x8 tile indexes.
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        1, 2, 7, 8, 7, 8, 7, 8,
        3, 4, 9,10, 9,10, 9,10,
        6, 6, 1, 2, 7, 8, 7, 8,
        6, 6, 3, 4, 9,10, 9,10,
     },
    // block: 11. TR inner corner.
    {
        // 8x8 tile indexes.
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        1, 2, 6, 6, 6, 6, 6, 6,
        3, 4, 6, 6, 6, 6, 6, 6,
        7, 8, 1, 2, 6, 6, 6, 6,
        9,10, 3, 4, 6, 6, 6, 6,
     },
    // block: 12. BR outer corner.
    {
        // 8x8 tile indexes.
        6, 6, 1, 2, 7, 8, 7, 8,
        6, 6, 3, 4, 9,10, 9,10,
        1, 2, 7, 8, 7, 8, 7, 8,
        3, 4, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
     },
    // block: 13. BR inner corner.
    {
        // 8x8 tile indexes.
        7, 8, 1, 2, 6, 6, 6, 6,
        9,10, 3, 4, 6, 6, 6, 6,
        1, 2, 6, 6, 6, 6, 6, 6,
        3, 4, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
     },
    // block: 14. Terrain.
    {
        // 8x8 tile indexes.
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
        7, 8, 7, 8, 7, 8, 7, 8,
        9,10, 9,10, 9,10, 9,10,
     },

    // block: 15
    {
        // 8x8 tile indexes.Left edge of the road. Start grid.
         8, 7, 8, 1, 2,11,12,11,
        10, 9,10, 3, 4,13,14,13,
         8, 7, 8, 1, 2,11,12,11,
        10, 9,10, 3, 4,13,14,13,
         8, 7, 8, 1, 2,11,12,11,
        10, 9,10, 3, 4,13,14,13,
         8, 7, 8, 1, 2,11,12,11,
        10, 9,10, 3, 4,13,14,13,
     },

    // block: 16
    {
        // 8x8 tile indexes. Right edge of the road. Start grid.
       12,11,12, 1, 2, 8, 7, 8,
       14,13,14, 3, 4,10, 9,10,
       12,11,12, 1, 2, 8, 7, 8,
       14,13,14, 3, 4,10, 9,10,
       12,11,12, 1, 2, 8, 7, 8,
       14,13,14, 3, 4,10, 9,10,
       12,11,12, 1, 2, 8, 7, 8,
       14,13,14, 3, 4,10, 9,10,
    },
    // block: 17
    {
        // 8x8 tile indexes.Left edge of the road. Light strip.
         8, 1, 2,15,15,15,15,15,
        10, 3, 4,15,15,15,15,15,
         8, 1, 2,15,15,15,15,15,
        10, 3, 4,15,15,15,15,15,
         8, 1, 2,15,15,15,15,15,
        10, 3, 4,15,15,15,15,15,
         8, 1, 2,15,15,15,15,15,
        10, 3, 4,15,15,15,15,15,
     },

    // block: 18
    {
        // 8x8 tile indexes. Right edge of the road. Light strip.
        15,15,15,15,15, 1, 2, 8,
        15,15,15,15,15, 3, 4,10,
        15,15,15,15,15, 1, 2, 8,
        15,15,15,15,15, 3, 4,10,
        15,15,15,15,15, 1, 2, 8,
        15,15,15,15,15, 3, 4,10,
        15,15,15,15,15, 1, 2, 8,
        15,15,15,15,15, 3, 4,10,
    },

    // block: 19. Road middle block, horizontal
    {
        // 8x8 tile indexes
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
        5, 5, 6, 6, 5, 5, 6, 6,
    },
    // block: 20. Road middle block, empty
    {
        // 8x8 tile indexes
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
    },
    // block: 21
    {
        // 8x8 tile indexes. Start grid, center block.
        12,11,12,11,12,11,12,11,
        14,13,14,13,14,13,14,13,
        12,11,12,11,12,11,12,11,
        14,13,14,13,14,13,14,13,
        12,11,12,11,12,11,12,11,
        14,13,14,13,14,13,14,13,
        12,11,12,11,12,11,12,11,
        14,13,14,13,14,13,14,13,
     },

    // block: 22. Road middle block with boost area
    {
        // 8x8 tile indexes
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6,15,15,15,15, 6, 6,
        6, 6,15,15,15,15, 6, 6,
        6, 6,15,15,15,15, 6, 6,
        6, 6,15,15,15,15, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
    },
    // block: 23. Road middle block with ramp
    {
        // 8x8 tile indexes
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6,16,16,16,16, 6, 6,
        6, 6,16,16,16,16, 6, 6,
        6, 6,16,16,16,16, 6, 6,
        6, 6,16,16,16,16, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
    },
};

const uint8_t billboard_object_bitmaps_count = 25 + 3;

const uint8_t* billboard_object_bitmaps [billboard_object_bitmaps_count] = {
    image_ship_1_a+2,
    image_ship_1_b+2,
    image_ship_1_c+2,
    image_ship_1_d+2,
    image_ship_1_e+2,
    image_ship_2_a+2,
    image_ship_2_b+2,
    image_ship_2_c+2,
    image_ship_2_d+2,
    image_ship_2_e+2,
    image_ship_3_a+2,
    image_ship_3_b+2,
    image_ship_3_c+2,
    image_ship_3_d+2,
    image_ship_3_e+2,
    image_ship_4_a+2,
    image_ship_4_b+2,
    image_ship_4_c+2,
    image_ship_4_d+2,
    image_ship_4_e+2,
    image_ship_5_a+2,
    image_ship_5_b+2,
    image_ship_5_c+2,
    image_ship_5_d+2,
    image_ship_5_e+2,
    //image_ship6+2,
    //image_pokitto+2,
    image_cactus+2,
    image_rock1+2,
    image_spot+2,
};

const uint8_t current_texture_bitmaps_count = 17;
const uint8_t* default_rom_bitmap_data[current_texture_bitmaps_count] =
{
    NULL,
    image_ball1_a+2,        /*index: 1*/
    image_ball1_b+2,        /*index: 2*/
    image_ball1_c+2,        /*index: 3*/
    image_ball1_d+2,        /*index: 4*/
    image_road1+2,          /*index: 5*/
    image_road2+2,          /*index: 6*/
    image_terrain_6_a+2,    /*index: 7*/
    image_terrain_6_b+2,    /*index: 8*/
    image_terrain_6_c+2,    /*index: 9*/
    image_terrain_6_d+2,    /*index: 10*/
    image_start_a+2,        /*index: 11*/
    image_start_b+2,        /*index: 12*/
    image_start_c+2,        /*index: 13*/
    image_start_d+2,        /*index: 14*/
    image_boost1+2,         /*index: 15*/
    image_ramp1+2,          /*index: 16*/
};

const uint8_t* current_texture_bitmap_data[current_texture_bitmaps_count] = {0};
const uint8_t* current_texture_bitmap_data_mm1[current_texture_bitmaps_count] = {0};
const uint8_t* current_texture_bitmap_data_mm2[current_texture_bitmaps_count] = {0};
