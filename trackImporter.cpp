#include "Pokitto.h"
#include "fix16.h"
#include "main.h"
#include "trackImporter.h"
#include "imageformat.h"
#include "objects_txt.h"
#include "track_txt.h"
#include "ship.h"

const char lineFeed = 10, carriageReturn=13;

bool TrackImporter::ReadFromFileAndValidateTrack(
    char* trackPath, char* trackDirName,
    /*OUT*/char* myTrack2, /*OUT*/char* trackName, /*OUT*/char* authorName )
{
    // Clear screen
    mygame.display.setColor(1,1);
    mygame.display.fillRect(0, 0, screenW, screenH);
    mygame.display.setColor(2,1);

    // Reset default physics and ship parameters in case they aren't set in track file
    CShip::ResetDefaultPhysicsParameters();
    CShip::ResetDefaultShipParameters();

    // Read the track ascii file from SD
    char myTrack1[mapTotalSizeinFile*2] = {0};
    //uint8_t blockMapRAM2[mapWidth*mapHeight];
    char filePathAndName[128] = {0};
    strcpy(filePathAndName, trackPath);
    #ifndef POK_SIM
    strcat(filePathAndName, "/");
    #endif
    strcat(filePathAndName, trackDirName);
    strcat(filePathAndName, "/");
    char* trackFileName = "track.txt";
    strcat(filePathAndName, trackFileName);
    uint8_t err = fileOpen(filePathAndName, FILE_MODE_READONLY);
    if(err)
    {
        mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        mygame.display.print(1, 50, "File is not");
        mygame.display.print(1, 60, "found!");
        fileClose(); // close any open files
        return false;
    }
    uint16_t len = fileReadBytes((uint8_t*)myTrack1, mapTotalSizeinFile*2);
    fileClose(); // close any open files

    // Read and validate the track.
    if(!ReadAndValidateTrack( myTrack1, len, trackDirName, trackFileName, /*OUT*/myTrack2, /*OUT*/trackName, /*OUT*/authorName )) {
        return false;
    }

    if(!ReadPhysicsParameters(myTrack1, len)) {
        return false;
    }
    return true;
}

bool TrackImporter::ReadPhysicsParameters(char* myTrack1, int32_t blockSize) {
    char* bufPtr = myTrack1;
    char* endPtr = myTrack1+blockSize;
    char* foundPtr = strstr(bufPtr, "physics]");
    if(foundPtr) {
        bufPtr = foundPtr+strlen("physics]");
        // Skip extra space, LF and CR chars
        while(bufPtr < endPtr && (*bufPtr == lineFeed || *bufPtr == carriageReturn || *bufPtr == ' ')) {
            bufPtr++;
        }

        while(bufPtr < endPtr && *bufPtr != '[') {
            // Physics parameters
            int32_t tileId;
            int32_t fr_s;
            int32_t fr_k;
            int32_t traction;
            int32_t rr;
            bufPtr = ReadValue(bufPtr, endPtr, /*OUT*/ tileId);
            bufPtr = ReadValue(bufPtr, endPtr, /*OUT*/ fr_s);
            bufPtr = ReadValue(bufPtr, endPtr, /*OUT*/ fr_k);
            bufPtr = ReadValue(bufPtr, endPtr, /*OUT*/ traction);
            bufPtr = ReadValue(bufPtr, endPtr, /*OUT*/ rr);

            // Set track parameters
            CShip::TileType tileType = CShip::enumTerrainTile;
            switch(tileId) {
                case 0: tileType = CShip::enumTrackTile; break;
                case 2: tileType = CShip::enumEdgeTile; break;
            }
            CShip::SetPhysicsParameters(tileType, fr_s*fix16_one/100, fr_k*fix16_one/100, traction, rr*fix16_one/100);
        }
    }
    return true;
}

bool TrackImporter::ReadFromROMAndValidateTrack(
    /*OUT*/char* myTrack2, /*OUT*/char* trackName, /*OUT*/char* authorName )
{
    // Clear screen
    mygame.display.setColor(1,1);
    mygame.display.fillRect(0, 0, screenW, screenH);
    mygame.display.setColor(2,1);

    CShip::ResetDefaultPhysicsParameters();
    CShip::ResetDefaultShipParameters();

    // Read the track ascii file from SD
    char myTrack1[mapTotalSizeinFile*2] = {0};

    // Read and validate the track.
    return ReadAndValidateTrack( (char*)track_txt, strlen(track_txt), "rom", "rom", /*OUT*/myTrack2, /*OUT*/trackName, /*OUT*/authorName );
}
//
////
bool TrackImporter::ReadAndValidateTrack( char* myTrack1, uint16_t len, char* trackDirName, char* trackFileName,
    /*OUT*/char* myTrack2, /*OUT*/char* trackName, /*OUT*/char* authorName )
{
    // If a file contains only the decimal bytes 9-13, 32-126, it's probably a pure ASCII text file.
    for(int32_t i=0; i<len; i++)
    {
        if(! ( (myTrack1[i]>=9 && myTrack1[i]<=13) || (myTrack1[i]>=32 && myTrack1[i]<=126) ) )
        {
            mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
            mygame.display.print(1, 50, "Not an ascii");
            mygame.display.print(1, 60, "file!");
            return false;
        }
    }

    // Skip extra LF and CR chars
    int32_t pos = 0;
    for(;myTrack1[pos]==lineFeed || myTrack1[pos]==carriageReturn;pos++);

    // Read the track name
    int32_t i=0;
    for(;myTrack1[pos]!=lineFeed && myTrack1[pos]!=carriageReturn && i<=maxTrackOrAuthorNameLen;pos++, i++)
        trackName[i] = myTrack1[pos];


    // Check for error.
    if(i > maxTrackOrAuthorNameLen-1)
    {
        mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        mygame.display.print(1, 50, "Circuit name");
        mygame.display.print(1, 60, "too long!");
        return false;
    }
    trackName[i]='\0';

    // Skip extra LF and CR chars
    for(;myTrack1[pos]==lineFeed || myTrack1[pos]==carriageReturn;pos++);

    // Read the author name
    i=0;
    for(;myTrack1[pos]!=lineFeed && myTrack1[pos]!=carriageReturn && i<=maxTrackOrAuthorNameLen;pos++,i++)
        authorName[i] = myTrack1[pos];

    // Check for error.
    if(i > maxTrackOrAuthorNameLen-1)
    {
        mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        mygame.display.print(1, 50, "Author name");
        mygame.display.print(1, 60, "too long!");
        return false;
    }
    authorName[i]='\0';

    // Skip extra LF and CR chars
    for(;myTrack1[pos]==lineFeed || myTrack1[pos]==carriageReturn;pos++);

    // Copy the map to the output array.
    int32_t currentPosInLine = 0;
    int32_t currentLineNum = 0;
    for(int32_t i=0; i<mapTotalSizeinFile && pos<len; i++, pos++)
    {
        char c = myTrack1[pos];

        if(c==lineFeed || c==carriageReturn )
        {
            //
            if(currentPosInLine<mapWidth)
            {
                // Too short line
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Line is too");
                mygame.display.print(1, 60, "short!");
                mygame.display.print(1, 70, "Line:");mygame.display.print(currentLineNum+3);
                return false;
            }

            // Next line.
            myTrack2[i] = lineFeed;
            currentPosInLine = 0;
            currentLineNum++;

            // Skip extra LF and CR chars
            pos++;
            for(;pos<len && (myTrack1[pos]==lineFeed || myTrack1[pos]==carriageReturn);pos++);
            pos--;
        }
        else if(currentPosInLine>=mapWidth)
        {
            // Too long line
            mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
            mygame.display.print(1, 50, "Line is too");
            mygame.display.print(1, 60, "long!");
            mygame.display.print(1, 70, "Line:");mygame.display.print(currentLineNum+3);
            return false;
        }

        if(currentLineNum>=mapHeight )
        {
            // Too many lines
            //mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            //mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
            //mygame.display.print(1, 50, "Too many");
            //mygame.display.print(1, 60, "lines!");
            //mygame.display.print(1, 70, "Line:");mygame.display.print(currentLineNum+3);
            //return false;
        }

        // If this is not the last char on the line, copy it to the map.
        if(myTrack2[i] != lineFeed)
        {
            // Check that the char is valid.
            int32_t convTableLen = sizeof(asciiTrackConversionTable);
            int32_t tableIndex=0;
            for(; tableIndex<convTableLen; tableIndex++ )
                if(asciiTrackConversionTable[tableIndex]==c)
                    break;
            if(tableIndex>=convTableLen || c==' ')
            {
                // Not a valid char.
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Invalid");
                mygame.display.print(1, 60, "char:\"");mygame.display.print(c);mygame.display.print("\"!");
                mygame.display.print(1, 70, "Line:");mygame.display.print(currentLineNum+3);
                mygame.display.print(1, 80, "Row:");mygame.display.print(currentPosInLine+1);
                return false;
            }

            // Store the char to the map.
            myTrack2[i] = c;
            currentPosInLine++;
        }

    }  // end for

    // Check that we found enough lines.
    if( currentLineNum<mapHeight-1 )
    {
        // Track file is too short
        mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        mygame.display.print(1, 50, "Too few lines");
        mygame.display.print(1, 60, "at line:");mygame.display.print(currentLineNum+3);
        return false;
    }

    // Check that the last line was not too short.
    if(currentPosInLine<mapWidth)
    {
        // Too short line
        //mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        //mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        //mygame.display.print(1, 50, "Line is too");
        //mygame.display.print(1, 60, "short!");
        //mygame.display.print(1, 70, "Line:");mygame.display.print(currentLineNum+3);
        //return false;
    }
    return true;
}

// Read the track objects ascii file from SD
bool TrackImporter::ReadTrackObjectsFromFile( char* trackPath, char* trackDirName )
{
    const int32_t blockSize = 1024;
    char buffer[blockSize+1] = {0};
    char filePathAndName[128] = {0};
    strcpy(filePathAndName, trackPath);
    #ifndef POK_SIM
    strcat(filePathAndName, "/");
    #endif
    strcat(filePathAndName, trackDirName);
    strcat(filePathAndName, "/");
    char* trackFileName = "objects.txt";
    strcat(filePathAndName, trackFileName);

    uint8_t err = fileOpen(filePathAndName, FILE_MODE_READONLY);
    if(err)
    {
        mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
        mygame.display.print(1, 40, trackFileName);mygame.display.setColor(2,1);
        mygame.display.print(1, 50, "File is not");
        mygame.display.print(1, 60, "found!");
        fileClose(); // close any open files
        return false;
    }

    // Read objects
    bool retval = ReadTrackObjects( buffer, blockSize, true );

    fileClose(); // close any open files

    return retval;
}

// Read the track objects ascii file from SD
bool TrackImporter::ReadTrackObjectsFromROM()
{
    // Read objects
    bool retval = ReadTrackObjects( (char*)objects_txt, strlen(objects_txt), false );

    fileClose(); // close any open files

    return retval;
}

// Read the track objects ascii file from SD
bool TrackImporter::ReadTrackObjects( char* buffer, int32_t blockSize, bool readFromFile )
{

    uint16_t len = blockSize;
    char* bufPtr = buffer ; //NULL;
    char* endPtr = buffer + len;
    bool isBufferLeft = false;
    if(readFromFile) FillBuffer( buffer, blockSize, /*OUT*/len, /*OUT*/&bufPtr,/*OUT*/&endPtr, /*OUT*/isBufferLeft );

    // *** "[waypoints]"
    char* foundPtr = strstr( bufPtr, "waypoints]" );
    if( foundPtr != NULL )
    {
       bufPtr =  foundPtr + strlen("waypoints]");

        for(; bufPtr < endPtr && (*bufPtr==lineFeed || *bufPtr==carriageReturn || *bufPtr==' '); bufPtr++); // Skip extra space, LF and CR chars
        int32_t wp = 0;
        while( ( wp < waypointMaxCount ) && ( *bufPtr != '[' ) )
        {
            // Check if we can read more of the buffer.
            if( bufPtr >= endPtr && isBufferLeft && readFromFile )
                FillBuffer( buffer, blockSize, /*OUT*/len, /*OUT*/&bufPtr,/*OUT*/&endPtr, /*OUT*/isBufferLeft );

            // Exit if in the end of the buffer.
            if( bufPtr >= endPtr)
                break;

            // Read waypoint data
            int32_t x;
            int32_t y;
            int32_t radius;
            int32_t vel;
            int32_t checkpoint;
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ x );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ y );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ radius );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ vel );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ checkpoint );

            // Add a new waypoint
            waypoints[wp].x = x;
            waypoints[wp].y = y;
            waypoints[wp].fxRadius = fix16_from_int(radius);
            waypoints[wp].fxTargetSpeed = fix16_from_int(vel);

            wp++;
        }
        waypointCount = wp;
    }

    // Check if we can read more of the buffer.
    if( bufPtr >= endPtr && isBufferLeft && readFromFile )
        FillBuffer( buffer, blockSize, /*OUT*/len, /*OUT*/&bufPtr,/*OUT*/&endPtr, /*OUT*/isBufferLeft );

    // *** "[billboards]"
    foundPtr = strstr( bufPtr, "billboards]" );
    if( foundPtr != NULL )
    {
        bufPtr =  foundPtr + strlen("billboards]");
        for(; bufPtr < endPtr && (*bufPtr==lineFeed || *bufPtr==carriageReturn || *bufPtr==' '); bufPtr++); // Skip extra space, LF and CR chars
        int32_t bbIndex = 0;
        while( ( bbIndex <= g_BillboardObjectArrayMaxCount - 8 ) && ( *bufPtr != '[' ) )
        {
            // Check if we can read more of the buffer.
            if( bufPtr >= endPtr && isBufferLeft && readFromFile)
                FillBuffer( buffer, blockSize, /*OUT*/len, /*OUT*/&bufPtr,/*OUT*/&endPtr, /*OUT*/isBufferLeft );

            // Exit if in the end of the buffer.
            if( bufPtr >= endPtr)
                break;

            // Read billboard data
            int32_t id;
            int32_t x;
            int32_t y;
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ id );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ x );
            bufPtr = ReadValue( bufPtr, endPtr, /*OUT*/ y );

            // Add a new billboard

            // "Cactus" or "Rock"
            const fix16_t fxCactusScaledSizeFactor = fix16_from_float(0.8);
            const uint8_t* cactus_bm = billboard_object_bitmaps[25];
            const fix16_t fxStoneScaledSizeFactor = fix16_from_float(1.0);
            const uint8_t* stone_bm = billboard_object_bitmaps[26];
            uint8_t* sprite_bm = (uint8_t*)cactus_bm;
            fix16_t fxSpriteScaledSizeFactor = fxCactusScaledSizeFactor;
            if( id==1 ) // stone
            {
                sprite_bm = (uint8_t*)stone_bm;
                fxSpriteScaledSizeFactor = fxStoneScaledSizeFactor;
            }
            const int16_t spriteBmW  = *(sprite_bm - 2);
            const int16_t spriteBmH  = *(sprite_bm - 1);

            // Init a billboard object.
            g_BillboardObjectArray[ bbIndex ].m_fxX = fix16_from_int( x );
            g_BillboardObjectArray[ bbIndex ].m_fxY = fix16_from_int( y );
            g_BillboardObjectArray[ bbIndex ].m_bitmap = sprite_bm;
            g_BillboardObjectArray[ bbIndex ].m_bitmapW = spriteBmW;
            g_BillboardObjectArray[ bbIndex ].m_bitmapH = spriteBmH;
            g_BillboardObjectArray[ bbIndex ].m_fxScaledWidth = spriteBmW * fxSpriteScaledSizeFactor;
            g_BillboardObjectArray[ bbIndex ].m_fxScaledHeight = spriteBmH * fxSpriteScaledSizeFactor;

            bbIndex++;
        }

        g_billboardObjectInRamCount = bbIndex;
    }

    return true;
}

void TrackImporter::FillBuffer( char* buffer, int32_t blockSize,
    /*OUT*/ uint16_t& len, /*OUT*/char** bufPtr,/*OUT*/char** endPtr, /*OUT*/bool& isBufferLeft )
{
    // First, copy the rest of the previous buffer to the beginning.
    int32_t remainingBufferLen = blockSize - len;
    if( remainingBufferLen > 0 )
        memcpy( buffer, buffer+len , remainingBufferLen);

    // Read the next block
    char* tmpPtr = buffer+remainingBufferLen;
    int32_t newLen = fileReadBytes((uint8_t*)tmpPtr, len);
    len = newLen + remainingBufferLen;  // total lenght

    // Check if the file is maximum lenght and cut it from the 'LF'
    *bufPtr = buffer;
    *endPtr = buffer + len;
    isBufferLeft = false;
    if(len == blockSize && buffer[len-1]!=lineFeed)
    {
        // Find previous LF
        char* ptr = *endPtr - 1;
        for(; ptr > *bufPtr && *ptr!=lineFeed; ptr--); // Go back to the nearest linefeed
        uint16_t len2 = *endPtr - ptr;
        len -= len2;
        *endPtr = buffer + len;
        isBufferLeft = true;
    }

     // Skip extra space, LF and CR chars
    for(; *bufPtr < *endPtr && (**bufPtr==lineFeed || **bufPtr==carriageReturn || **bufPtr==' '); (*bufPtr)++);
}

char* TrackImporter::ReadValue( char* bufPtr, char* endPtr, /*OUT*/ int32_t& value )
{
    //
    // Read value.

    // find the nearesr of: comma, lineFeed or carriageReturn.
    char* newBufPtr = strchr( bufPtr, ',');
    char* newBufPtr2 = strchr( bufPtr, lineFeed);
    if(newBufPtr==NULL) newBufPtr = newBufPtr2;
    if( newBufPtr2!=NULL && newBufPtr2 < newBufPtr) newBufPtr = newBufPtr2;
    newBufPtr2 = strchr( bufPtr, carriageReturn);
    if( newBufPtr2!=NULL && newBufPtr2 < newBufPtr) newBufPtr = newBufPtr2;

    // if found, convert to integer.
    if(!newBufPtr || newBufPtr-bufPtr >= 8) return endPtr;
    char valueAsStr[8];
    int32_t strsize = newBufPtr-bufPtr;
    strncpy( valueAsStr, bufPtr, strsize );
    valueAsStr[ strsize ] = 0;
    value = atoi( valueAsStr );

    // Skip trailing comma, space, LF and CR chars
    bufPtr += strsize;
    for(; bufPtr < endPtr && (*bufPtr==lineFeed || *bufPtr==carriageReturn || *bufPtr==',' || *bufPtr==' '); bufPtr++);

    return bufPtr;
}

//
bool TrackImporter::ReadAndValidateTextures(char* trackDirPath, char* trackDirName)
{
    RestoreRomTextures();

    struct STextureFileParam
    {
        int32_t type; // 0=texture, 1=sprite, 2=background tile
        int32_t w,h;
        int32_t numOfParts;
        char filename[16];
    };

    const int32_t startOfSpritesIndex = 8;
    const STextureFileParam textureFileParamArr[] =
    {
        {0,0,0,0,""},
        {0,16,24,4,"tex01.bmp"},  // 1: ball
        {0,8,12,1,"tex02.bmp"},   // 2: road1
        {0,8,12,1,"tex03.bmp"},   // 3: road2
        {0,16,24,4,"tex04.bmp"},  // 4: terrain
        {0,16,24,4,"tex05.bmp"},  // 5: start grid
        {0,8,12,1,"tex06.bmp"},   // 6: booster
        {0,8,12,1,"tex07.bmp"},   // 7: ramp

        {1,-1,-1,1,"sprite01.bmp"},  // Cactus
        {1,-1,-1,1,"sprite02.bmp"},  // Stone

        {2,32,16,1,"bkground.bmp"},   // background tile
    };

    // Try to read all textures.
    char filePathAndName[128] = {0};
    int32_t textureFileParamArrSize = sizeof(textureFileParamArr) / sizeof(textureFileParamArr[0]);
    const int32_t startOfPaletteImportIndex = 41;
    for(int32_t fileIndex=1, texIndex=1; fileIndex < textureFileParamArrSize; fileIndex++,texIndex++)
    {

        //
        char fileName[16];
        strcpy(fileName, textureFileParamArr[fileIndex].filename);
        strcpy(filePathAndName, trackDirPath);
        #ifndef POK_SIM
        strcat(filePathAndName, "/");
        #endif
        strcat(filePathAndName, trackDirName);
        strcat(filePathAndName, "/");
        strcat(filePathAndName, fileName);

        // Read texture file.
        uint16_t* palette2 = NULL; // Gets the ownership.
        uint8_t* bitmap = NULL; // Gets the ownership.
        int err = openImageFileFromSD(filePathAndName, /*OUT*/&palette2, /*OUT*/&bitmap);
        if(!err && fileIndex==1)
        {
            // Append the palette to the end of current palette.
            for(int32_t c=startOfPaletteImportIndex, ii=0; c < 255; c++,ii++)
                Pokitto::Display::palette[c] = palette2[ii];

            // Copy the default palette.
            memcpy((uint8_t*)g_gamePalette, (uint8_t*)Pokitto::Display::palette, 256*2);
            //Pokitto::Core::display.load565Palette((const uint16_t*)g_gamePalette);
        }
        free(palette2); palette2 = NULL;

        //
        if(err == -1 || err == -2)
        {
            // Not found, skip texture.
            if( textureFileParamArr[ fileIndex].numOfParts == 4 )
                texIndex+=3;
        }
//        else if(err == -2)
//        {
//            // Cannot open
//            mygame.display.setColor(3,1);mygame.display.print(1, 30, fileName);mygame.display.setColor(2,1);
//            mygame.display.print(1, 40, "Cannot");
//            mygame.display.print(1, 50, "open image.");
//            return false;
//        }
        else if(err == -6)
        {
            // Wrong width
            if (textureFileParamArr[fileIndex].w == -1 && textureFileParamArr[fileIndex].h == -1) // sprite
            {
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Width not");
                mygame.display.print(1, 60, "divisible");
                mygame.display.print(1, 60, "by 4");
            }
            else
            {
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Width should");
                mygame.display.print(1, 60, "be: ");mygame.display.print(textureFileParamArr[ fileIndex].w);
            }

            return false;
        }
        else if(err == -10)
        {
            // Wrong palette size
            mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
            mygame.display.print(1, 50, "Color count");
            mygame.display.print(1, 60, "not 256.");
            return false;
        }
        else if(err == -12)
        {
            // OOM
            mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
            mygame.display.print(1, 50, "Out of");
            mygame.display.print(1, 60, "memory.");
            return false;
        }
        else if(err != 0)
        {
            // Other error
            mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
            mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
            mygame.display.print(1, 50, "Invalid");
            mygame.display.print(1, 60, "image format");
            return false;
        }
        else if ( textureFileParamArr[fileIndex].type == 1 ) // sprite
        {
            // Billboard sprite ok.

            // Convert indexes
            int32_t w = bitmap[0];
            int32_t h = bitmap[1];
            uint8_t* bitmapData = &(bitmap[2]);
            for(int32_t ii=0; ii<w*h; ii++)
            {
                uint8_t colorIndex = bitmapData[ii];
                if(Pokitto::Display::palette[0] == Pokitto::Display::palette[colorIndex+startOfPaletteImportIndex])
                    colorIndex = 0;  // Transparent color
                else
                    colorIndex += startOfPaletteImportIndex;  // Convert to the imported color area.

                *(bitmapData+ii) = colorIndex;
            }
            g_spriteBitmaps[fileIndex-startOfSpritesIndex] = bitmap;
            bitmap = NULL;
        }
        else if ( textureFileParamArr[fileIndex].type == 2 ) // background tile
        {
            // Billboard sprite ok.

            // Convert indexes
            int32_t w = textureFileParamArr[ fileIndex].w;
            int32_t h = textureFileParamArr[ fileIndex].h;
            uint8_t* bitmapData = &(bitmap[2]);
            for(int32_t ii=0; ii<w*h; ii++)
            {
                uint8_t colorIndex = bitmapData[ii] + startOfPaletteImportIndex;  // Convert to the imported color area.;
                *(bitmapData+ii) = colorIndex;
            }
            g_BackgroundTileBitmap = bitmap;
            bitmap = NULL;
        }
        else if( textureFileParamArr[fileIndex].type == 0 )
        {
            // Texture ok.

            int32_t w = bitmap[0];
            int32_t h = bitmap[1];

            // Check size
            if( textureFileParamArr[ fileIndex].w !=  w)
            {
                // Wrong width
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Width should");
                mygame.display.print(1, 60, "be: ");mygame.display.print(textureFileParamArr[ fileIndex].w);
                mygame.display.print(1, 70, "Is:");mygame.display.print(w);
                free(bitmap);
                return false;
            }
            if( textureFileParamArr[ fileIndex].h !=  h)
            {
                // Wrong height
                mygame.display.setColor(3,1);mygame.display.print(1, 30, trackDirName);
                mygame.display.print(1, 40, fileName);mygame.display.setColor(2,1);
                mygame.display.print(1, 50, "Height");
                mygame.display.print(1, 60, "should");
                mygame.display.print(1, 70, "be: ");mygame.display.print(textureFileParamArr[ fileIndex].h);
                mygame.display.print(1, 80, "Is:");mygame.display.print(h);
                free(bitmap);
                return false;
            }

            if( textureFileParamArr[ fileIndex].numOfParts == 1 )
            {
                current_texture_bitmap_data[texIndex] = bitmap+2;
                current_texture_bitmap_data_mm1[texIndex] =  current_texture_bitmap_data[texIndex] + (w * w);
                current_texture_bitmap_data_mm2[texIndex] =  current_texture_bitmap_data[texIndex] + (w * w) + (w>>1);
                bitmap = NULL;

                // Convert indexes
                for(int32_t ii=0; ii<w*h; ii++)
                    *(((uint8_t*)(current_texture_bitmap_data[texIndex]))+ii) = current_texture_bitmap_data[texIndex][ii] + startOfPaletteImportIndex;
            }
            else if( textureFileParamArr[ fileIndex].numOfParts == 4 )
            {
                // 16x24
                //
                uint8_t* tmpBitmap0 = (uint8_t *) malloc((8*12) + 2);  // header takes 2 bytes
                uint8_t* tmpBitmap1 = (uint8_t *) malloc((8*12) + 2);  // header takes 2 bytes
                uint8_t* tmpBitmap2 = (uint8_t *) malloc((8*12) + 2);  // header takes 2 bytes
                uint8_t* tmpBitmap3 = (uint8_t *) malloc((8*12) + 2);  // header takes 2 bytes
                tmpBitmap0[0] = 8; tmpBitmap0[1] = 12; // width & height
                tmpBitmap1[0] = 8; tmpBitmap1[1] = 12; // width & height
                tmpBitmap2[0] = 8; tmpBitmap2[1] = 12; // width & height
                tmpBitmap3[0] = 8; tmpBitmap3[1] = 12; // width & height
                uint8_t* tmpBitmapData0 = &(tmpBitmap0[2]);
                uint8_t* tmpBitmapData1 = &(tmpBitmap1[2]);
                uint8_t* tmpBitmapData2 = &(tmpBitmap2[2]);
                uint8_t* tmpBitmapData3 = &(tmpBitmap3[2]);
                uint8_t* readBitmapData = &(bitmap[2]);
                for(int32_t x=0; x<8; x++)
                {
                    for(int32_t y=0; y<8; y++)
                    {
                        tmpBitmapData0[y*8+x] = readBitmapData[y*16+x] + startOfPaletteImportIndex;
                        tmpBitmapData1[y*8+x] = readBitmapData[y*16+x+8] + startOfPaletteImportIndex;
                        tmpBitmapData2[y*8+x] = readBitmapData[(y+8)*16+x] + startOfPaletteImportIndex;
                        tmpBitmapData3[y*8+x] = readBitmapData[(y+8)*16+x+8] + startOfPaletteImportIndex;
                    }
                }
                current_texture_bitmap_data[texIndex] = tmpBitmapData0;
                current_texture_bitmap_data[texIndex+1] = tmpBitmapData1;
                current_texture_bitmap_data[texIndex+2] = tmpBitmapData2;
                current_texture_bitmap_data[texIndex+3] = tmpBitmapData3;

                // MIPMAP 1 (4x4 pixels)
                uint8_t* readBitmapDataMM = readBitmapData + (16*16);
                for(int32_t x=0; x<4; x++)
                {
                    for(int32_t y=0; y<4; y++)
                    {
                        tmpBitmapData0[(y+8)*8+x] = readBitmapDataMM[y*16     + x] + startOfPaletteImportIndex;
                        tmpBitmapData1[(y+8)*8+x] = readBitmapDataMM[y*16     + x+4] + startOfPaletteImportIndex;
                        tmpBitmapData2[(y+8)*8+x] = readBitmapDataMM[(y+4)*16 + x] + startOfPaletteImportIndex;
                        tmpBitmapData3[(y+8)*8+x] = readBitmapDataMM[(y+4)*16 + x+4] + startOfPaletteImportIndex;
                    }
                }
                current_texture_bitmap_data_mm1[texIndex] = tmpBitmapData0 + (8 * 8);
                current_texture_bitmap_data_mm1[texIndex+1] = tmpBitmapData1 + (8 * 8);
                current_texture_bitmap_data_mm1[texIndex+2] = tmpBitmapData2 + (8 * 8);
                current_texture_bitmap_data_mm1[texIndex+3] = tmpBitmapData3 + (8 * 8);

                // MIPMAP 2 (2x2 pixels)
                uint8_t* readBitmapDataMMM = readBitmapDataMM + 8;
                for(int32_t x=0; x<2; x++)
                {
                    for(int32_t y=0; y<2; y++)
                    {
                        tmpBitmapData0[(y+8)*8+x+4] = readBitmapDataMMM[y*16     + x] + startOfPaletteImportIndex;
                        tmpBitmapData1[(y+8)*8+x+4] = readBitmapDataMMM[y*16     + x+2] + startOfPaletteImportIndex;
                        tmpBitmapData2[(y+8)*8+x+4] = readBitmapDataMMM[(y+2)*16 + x] + startOfPaletteImportIndex;
                        tmpBitmapData3[(y+8)*8+x+4] = readBitmapDataMMM[(y+2)*16 + x+2] + startOfPaletteImportIndex;
                    }
                }
                current_texture_bitmap_data_mm2[texIndex] = tmpBitmapData0 + (8 * 8) + 4;
                current_texture_bitmap_data_mm2[texIndex+1] = tmpBitmapData1 + (8 * 8) + 4;
                current_texture_bitmap_data_mm2[texIndex+2] = tmpBitmapData2 + (8 * 8) + 4;
                current_texture_bitmap_data_mm2[texIndex+3] = tmpBitmapData3 + (8 * 8) + 4;

                texIndex+=3;

                //
                free(bitmap); bitmap = NULL;
            }
        }
    }  // end for

    return true;
}

void TrackImporter::ConvertAsciiToMapElements( char* myTrack2 )
{
    uint8_t blockMapRAM2[mapWidth*mapHeight];

    // Map of blocks. Defines the whole game field!
    if( blockMapRAM == NULL )
        blockMapRAM = new uint8_t[mapWidth*mapHeight];
    int32_t convTableLen = sizeof(TrackImporter::asciiTrackConversionTable);
    g_billboardObjectInRamCount = 0;
    for(int32_t y = 0; y < mapHeight; y++)
    {
        for(int32_t x = 0; x < mapWidth; x++)
        {
            // Create map
            int invY = mapHeight - 1 - y; // mirror map vertically
            int32_t mapWidth2 = mapWidth+1; // added newline
            char item = myTrack2[invY*mapWidth2 + x];
            //assert(item!=' ');

            // Search the item from the conversion table
            int32_t i=0;
            for(; i<convTableLen; i++ )
                if(TrackImporter::asciiTrackConversionTable[i]==item)
                    break;

            if(i>=convTableLen || item==' ')
                break; // error

            blockMapRAM[y*mapWidth + x] = i;

        }  // end for

    }  // end for
}
