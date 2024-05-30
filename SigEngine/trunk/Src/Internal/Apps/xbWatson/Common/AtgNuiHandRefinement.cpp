
// ********************************************************************************************************* //
// --------------------------------------------------------------------------------------------------------- //
//
// Copyright (c) Microsoft Corporation
//
// --------------------------------------------------------------------------------------------------------- //
// ********************************************************************************************************* //

#include "stdafx.h"
#include "AtgNuiHandRefinement.h"
#include <nuiapi.h>
#include <math.h>

// ********************************************************************************************************* //
//
//  Namespace:
//

namespace ATG
{

// The search diretions used to walk along the arm.  The vector between the elbow and 
// the hand can be wrong. It is mostly correct.  These directions are used to search the 8
// cardinal directions and diagonals that correspond to a give elbow hand direction.
enum SearchDirections
{
    SEARCH_FORWARD,
    SEARCH_DIAGONAL_LEFT,
    SEARCH_DIAGONAL_RIGHT,
    SEARCH_ORTHOGONAL_LEFT,
    SEARCH_ORTHOGONAL_RIGHT,
    SEARCH_DIRECTION_COUNT
};
// In the final stage of hand refinement, the 320x240 depth map is used to 
// find the refined values.  The pixels that are deemed hand pixels are then averaged
// to vote for the final hand position.
// In order to threshold the pixels in Z, the min values must be found.  
// Due to noise in the depth map, the neighboring value are also checked.  If they are within
// a threshhold then the min value is not noise.
// 25 is used based on empirical analasis of depth map noise. 
static CONST SHORT g_uAcceptableToleranceForNeighborsOfMinValue = 25;

// When searching for the closest pixels huge outliers resulting from bad camera noise should be removed.
static CONST INT g_iRemoveLargeOutlierThreshold = 300;

// This tolerance is used by multiple portions of the algorithm.
// When the hand is in front of the body, the max value in the depth buffer is calculate by adding 
// 8 centemeters to the min value. 
static CONST INT g_iThresholdToSearchPastTheHand = 80;

// This tolerance is used by the code to walk to the end of the arm.  The theshold is smaller than 
// g_iThresholdToSearchPastTheHand as we're more sensative to walking off of the arm.
static CONST INT g_iThresholdToSearchPastTheHandStrict = 40;

// The number of pixels along the edge of the search rectangle that must be erroneously selected 
// as hand pixels to convince he algorithm to fall back to raw data.
static CONST INT g_iPixelsOnEdgesOutof12ToRevert = 8;

// When finding depth from the depth map a value of 300 is sufficiently big to tell us
// we've walked off of the hand onto something else.
static CONST INT g_iAcceptableRawDepthMapDifference = 300;

// When walking to the end of the hand. we're looking for small changes as we're trying to stay on the arm.
// If the change is greater than 2.5 centemeters then we don't want to follow that direction.
static CONST INT g_iWalktoEndofHandTheshold = 40;

// When detecting if the arm is close to the body, use this threshold to find runs
static CONST INT g_iWalkOnBodyThreshold = 80;

// the voxel mask only adds voxels within this threshold
static CONST INT g_iBuildVoxelMaskTheshold = 80;

// Number of 80x60 pixels to move when searching for the hand.
static CONST INT g_nWalkOntoHandSearchRadius = 1;

// Number of iterations to walk along the arm in the 80x60 map.
static CONST INT g_nWalkToEdgeIterations = 9; 
//static CONST INT g_nWalkToEdgeIterations = 16; 

// global shift right VMX Vector
static CONST __vector4 g_v8ChannelShiftRight3 = __vspltish( 3 );

// global remove segmentation mask VMX vector
static CONST __vector4 g_v8ChannelAnd7 = __vspltish( 7 ); 

// - in y is up
// The search directions for the 8 surrounding pixels
static CONST INT g_SpiralSearchOffsets[][2] =
{
    {-1, 0},
    {-1, -1},
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1}
};

// Do it branchless on PPC
// These are branchless instructions to avoid branch mispredictions
// Determine the difference between them.
// diff = (a<b) ? (a-b) : 0;
// a-(a-b) = b; // Flip if (a<b)
// b+(a-b) = a; // Flip if (a>b)
#define GETMIN(dst,a,b) diff = a-b; mask = diff>>31; diff = diff&mask; dst = b+diff    

#ifndef MAX_REAL_DEPTH
#define MAX_REAL_DEPTH  4999
#endif
#ifndef MIN_REAL_DEPTH
#define MIN_REAL_DEPTH  50
#endif


// This structure is used by the octagon search it holds an average X,Y, and Depth.
// It also holds the number of values that went into calculating that average.
struct OctagonAverageSearch 
{
    INT3Vector iCellAverageDepth[8];
    INT iValuesInAverage[8];  

};


struct TemporaryStackData
{
    BOOL bValid80x60Voxels[12][12];
    INT iVoxelSearchRadius;
    INT iSquaredSizeOfHandTheshold;
};

//-----------------------------------------------------------------------------
// Clamps values to be valid lookup valus in a 80x60 map
//-----------------------------------------------------------------------------
inline void Clamp80x60( INT* pX, INT* pY )
{
    *pX = max( 0, min( 79, *pX ) );
    *pY = max( 0, min( 59, *pY ) );
}

//-----------------------------------------------------------------------------
// Transforms from world coordinates to camera coordinates.
//
// - w: The world coordinates (in milimeters)
// - screenY: The x coordinate in camera space
// - screenY: The y coordinate in camera space
//-----------------------------------------------------------------------------
inline void TransformWorldToScreen320x240(
                                   _In_ XMVECTOR w, 
                                   _In_ FLOAT FOV_X_DEGREES, 
                                   _Out_ INT* pScreenX, 
                                   _Out_ INT* pScreenY)
{
    static FLOAT TAN_FOV_PER_PIXEL_INV = ( (320.0f)/2.0f ) / tan( XMConvertToRadians( 58.51213528192006345094411371771f * 0.5f ) );
    *pScreenX = (INT)( ( w.x * TAN_FOV_PER_PIXEL_INV ) /w.z + (320.0f)/2.0f );
    *pScreenY = (INT)( ((FLOAT)240.0f)/2.0f - ( w.y * TAN_FOV_PER_PIXEL_INV ) /w.z );
}

//--------------------------------------------------------------------------------------
// Name: TransformWorldToScreen80x60
// Desc: Transform a world pace coordinate to a depth map coordinate with a depth value
//       
//--------------------------------------------------------------------------------------
inline void TransformWorldToScreen80x60(
                                   _In_ XMVECTOR w, 
                                   _Out_ INT3Vector* pCoord )
{
    static FLOAT TAN_FOV_PER_PIXEL_INV = ( (80.0f)/2.0f ) / 
        tan( XMConvertToRadians( NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * 0.5f ) );
    pCoord->iX = (INT)( ( w.x * TAN_FOV_PER_PIXEL_INV ) /w.z + (80.0f)/2.0f );
    pCoord->iY = (INT)( ((FLOAT)60.0f)/2.0f - ( w.y * TAN_FOV_PER_PIXEL_INV ) /w.z );
    pCoord->iZ = (INT)( XMVectorGetZ( w ) * 1000.0f);
}

//-----------------------------------------------------------------------------
// Transforms from camera to world coordinates. The world coordinates are 
// represented in milimeters.
//
// - screenX: The x coordinate
// - screenY: The y coordinate
// - screenZ: The depth
// - pWX:     The x position of the point in world space
// - pWY:     The y position of the point in world space
// - pWZ:     The z position of the point in world space
//-----------------------------------------------------------------------------
inline void TransformScreenToWorld320x240(
                                   _In_ FLOAT screenX, 
                                   _In_ FLOAT screenY, 
                                   _In_ FLOAT screenZ, 
                                   _Out_ XMVECTOR* pW)
{
    static const FLOAT TAN_FOV_PER_PIXEL = 
        tan( XMConvertToRadians( NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * 0.5f ) ) / ( ((FLOAT)320)/2.0f);

    pW->x = ( ( (FLOAT)screenX - ((FLOAT)320)/2.0f ) * (FLOAT)screenZ * TAN_FOV_PER_PIXEL );
    pW->y = ( ( ((FLOAT)240)/2.0f - (FLOAT)screenY ) * (FLOAT)screenZ * TAN_FOV_PER_PIXEL );
    pW->z = (FLOAT)screenZ;
    pW->w = 1.0f;
}

//-----------------------------------------------------------------------------
// Transforms from camera to world coordinates. The world coordinates are 
// represented in milimeters.
//
// - screenX: The x coordinate
// - screenY: The y coordinate
// - screenZ: The depth
// - pWX:     The x position of the point in world space
// - pWY:     The y position of the point in world space
// - pWZ:     The z position of the point in world space
//-----------------------------------------------------------------------------
inline void TransformScreenToWorld80x60(
                                   _In_ FLOAT screenX, 
                                   _In_ FLOAT screenY, 
                                   _In_ FLOAT screenZ, 
                                   _Out_ XMVECTOR* pW)
{
    static const FLOAT TAN_FOV_PER_PIXEL = 
        tan( XMConvertToRadians( NUI_CAMERA_DEPTH_NOMINAL_HORIZONTAL_FOV * 0.5f ) ) / ( ((FLOAT)80)/2.0f);

    pW->x = ( ( (FLOAT)screenX - ((FLOAT)80)/2.0f ) * (FLOAT)screenZ * TAN_FOV_PER_PIXEL );
    pW->y = ( ( ((FLOAT)60)/2.0f - (FLOAT)screenY ) * (FLOAT)screenZ * TAN_FOV_PER_PIXEL );
    pW->z = (FLOAT)screenZ;
    pW->w = 1.0f;
}

//--------------------------------------------------------------------------------------
// Name: Get3x3Average80x60
// Desc: This function returns the average depth for a 3x3 grid in the 80 x 60 depth map
//--------------------------------------------------------------------------------------
INT Get3x3Average80x60( USHORT* pDepth80x60, 
                       INT iShortPitch80x60, 
                       INT3Vector iInSearchLocation,
                       INT *piAverageDepth )
{

    INT iRangeX[] = {
        max( 0, iInSearchLocation.iX - 1 ),
        min( 80, iInSearchLocation.iX + 2 ) };
    INT iRangeY[] = {
        max( 0, iInSearchLocation.iY - 1 ),
        min( 60, iInSearchLocation.iY + 2 ) };

    INT iTotal = 0;
    INT iGoodValues = 0;
    for ( INT iY = iRangeY[0]; iY < iRangeY[1]; ++iY )
    {
        for ( INT iX = iRangeX[0]; iX < iRangeX[1]; ++iX )
        {
            assert( iX < 80 );
            assert( iY < 60 );
            USHORT uVal = pDepth80x60[ iX + iY * iShortPitch80x60 ];
            USHORT uDepth = uVal >> 3;
            if ( uDepth != 0 ) 
            {
                ++iGoodValues;
                iTotal += (INT)uDepth;
            }
        }
    }
    if ( iTotal == 0 ) return 0;

    *piAverageDepth = iTotal / iGoodValues;
    return iGoodValues;
}

//--------------------------------------------------------------------------------------
// Name: Get3x3Average80x60
// Desc: This function returns the average depth for a 3x3 grid in the 80 x 60 depth map
//--------------------------------------------------------------------------------------
INT Get3x3Average80x60WithPlayerIndex( USHORT* pDepth80x60, 
                       INT iShortPitch80x60, 
                       INT3Vector iInSearchLocation,
                       USHORT uPlayerIndex,
                       INT *piAverageDepth )
{

    INT iRangeX[] = {
        max( 0, iInSearchLocation.iX - 1 ),
        min( 80, iInSearchLocation.iX + 2 ) };
    INT iRangeY[] = {
        max( 0, iInSearchLocation.iY - 1 ),
        min( 60, iInSearchLocation.iY + 2 ) };

    INT iTotal = 0;
    INT iGoodValues = 0;
    for ( INT iY = iRangeY[0]; iY < iRangeY[1]; ++iY )
    {
        for ( INT iX = iRangeX[0]; iX < iRangeX[1]; ++iX )
        {
            assert( iX < 80 );
            assert( iY < 60 );
            USHORT uVal = pDepth80x60[ iX + iY * iShortPitch80x60 ];
            USHORT uDepth = uVal >> 3;
            USHORT uIndex = uVal & 0x0007;
            if ( uDepth != 0 && uIndex == uPlayerIndex ) 
            {
                ++iGoodValues;
                iTotal += (INT)uDepth;
            }
        }
    }
    if ( iTotal == 0 ) return 0;

    *piAverageDepth = iTotal / iGoodValues;
    return iGoodValues;
}


//--------------------------------------------------------------------------------------
// Name: Get3x3AverageOnArm80x60
// Desc: This function returns the average depth for a 3x3 grid in the 80 x 60 depth map
// The values is thresholded and tested against the player index.
// The X,Y, and Z average is returned.
//--------------------------------------------------------------------------------------
INT Get3x3AverageOnArm80x60( USHORT* pDepth80x60, 
                       INT iShortPitch80x60, 
                       INT3Vector iInSearchLocation, 
                       USHORT uPlayer, 
                       INT iNearPad,
                       INT iFarPad,
                       INT3Vector *piAverageDepth )
{

    // clamp so we don't index out side of the array.
    INT iRangeX[] = {
        max( 0, iInSearchLocation.iX - 1 ), // 3x3 kernel check left element is not smaller than 0
        min( 80, iInSearchLocation.iX + 2 ) }; // 3x3 kernel check right element is less than 79
    INT iRangeY[] = {
        max( 0, iInSearchLocation.iY - 1 ),
        min( 60, iInSearchLocation.iY + 2 ) };

    INT iGoodValues = 0;
    piAverageDepth->iX = 0;
    piAverageDepth->iY = 0;
    piAverageDepth->iZ = 0;
    for ( INT iY = iRangeY[0]; iY < iRangeY[1]; ++iY )
    {
        for ( INT iX = iRangeX[0]; iX < iRangeX[1]; ++iX )
        {
            assert( iX < 80 );
            assert( iY < 60 );
            USHORT uVal = pDepth80x60[ iX + iY * iShortPitch80x60 ];
            USHORT uDepth = uVal >> 3;
            if ( uDepth != 0 ) 
            {
                ++iGoodValues;
                piAverageDepth->iZ += (INT)uDepth;
                piAverageDepth->iX += iX;
                piAverageDepth->iY += iY;
            }
        }
    }
    if ( iGoodValues == 0 ) return 0;

    piAverageDepth->iZ /= iGoodValues;
    piAverageDepth->iX /= iGoodValues;
    piAverageDepth->iY /= iGoodValues;

    assert( iInSearchLocation.iX < 80 );
    assert( iInSearchLocation.iY < 60 );
    USHORT uCenterValue = pDepth80x60[ iInSearchLocation.iX + iInSearchLocation.iY * iShortPitch80x60 ];
    INT iDepth = uCenterValue >> 3;
    INT iIndex = uCenterValue & 0x0007;
    if ( iIndex != uPlayer || abs(iDepth - piAverageDepth->iZ) > g_iRemoveLargeOutlierThreshold ) 
    {
        return 0;
    }
    
    return iGoodValues;
}

//--------------------------------------------------------------------------------------
// Name: FindAverageForHand320x240
// Desc: This function returns the average from the 320x240 depth map.
//--------------------------------------------------------------------------------------
INT FindAverageForHand320x240 ( USHORT *pDepth320x240, 
                               DWORD iShortPitch320x240,
                               USHORT *pDepth80x60, 
                               DWORD iShortPitch80x60, 
                               RefinementData* pRefinementData,
                               HandSpecificData* pHandSpecificData,
                               XMVECTOR* pRefinedHand,
                               TemporaryStackData* pTSD
                              )
{
    USHORT uPlayerIndex = pRefinementData->m_uSegmentationIndex;

    INT iAverageX = 0;
    INT iAverageY = 0;
    INT iAverageZ = 0;
    INT iTotalCount = 0;

    INT3Vector iRefinedPosition80x60 = pHandSpecificData->INT3ScreenSpaceHandRefined80x60;
   // to do the bound checks ahead of time for a speed up
    INT3Vector iSearchLocation320x240 = pHandSpecificData->INT3ScreenSpaceHandRefined80x60;
    iSearchLocation320x240.iX *= 4;
    iSearchLocation320x240.iY *= 4;


#ifdef _DEBUG
    memset( &pHandSpecificData->m_VisualizeHandFramesData, 2, sizeof (VisualizeHandFramesData) );
#endif

    INT iTopThreeSmallest[] = {10000, 10000, 10000};    

    INT2Range XRange, YRange;
    XRange.iMin = 0;
    XRange.iMax = 12;
    YRange.iMin = 0;
    YRange.iMax = 12;
    const INT iNumSearchPixels = 5;

    if ( iRefinedPosition80x60.iX < iNumSearchPixels )
    {
        XRange.iMin = iNumSearchPixels - iRefinedPosition80x60.iX;
    }
    if ( iRefinedPosition80x60.iX > ( 80 + iNumSearchPixels - XRange.iMax ) )
    {
        XRange.iMax = 80 + iNumSearchPixels - iRefinedPosition80x60.iX;
    }
    if ( iRefinedPosition80x60.iY < iNumSearchPixels )
    {
        YRange.iMin = iNumSearchPixels - iRefinedPosition80x60.iY;
    }
    if ( iRefinedPosition80x60.iY > ( 60 + iNumSearchPixels - YRange.iMax ) )
    {
        YRange.iMax = 60 + iNumSearchPixels - iRefinedPosition80x60.iY;
    }

    for ( INT iY12x12 = YRange.iMin; iY12x12 < YRange.iMax; ++iY12x12 )
    {
        for ( INT iX12x12 = XRange.iMin; iX12x12 < XRange.iMax; ++iX12x12 )
        {             
            if ( pTSD->bValid80x60Voxels[iY12x12][iX12x12] )
            {
                INT iX80x60 = iX12x12 - iNumSearchPixels + iRefinedPosition80x60.iX;
                INT iY80x60 = iY12x12 - iNumSearchPixels + iRefinedPosition80x60.iY;

                assert( iX80x60 < 80 );
                assert( iY80x60 < 60 );

                INT iValue = pDepth80x60[iX80x60 + iY80x60 * iShortPitch80x60];
                INT iDepth = iValue >> 3;
                INT iIndex = iValue & 0x0007;
                if ( iIndex == uPlayerIndex )
                {
                    if ( iDepth < iTopThreeSmallest[0] ) 
                    {
                        iTopThreeSmallest[2] = iTopThreeSmallest[1];
                        iTopThreeSmallest[1] = iTopThreeSmallest[0];
                        iTopThreeSmallest[0] = iDepth;
                    }
                    else if ( iDepth < iTopThreeSmallest[1] ) 
                    {       
                        iTopThreeSmallest[2] = iTopThreeSmallest[1];
                        iTopThreeSmallest[1] = iDepth;
                    }
                    else if ( iDepth < iTopThreeSmallest[2] ) 
                    {
                        iTopThreeSmallest[2] = iDepth;
                    }
                }
            }
        }
    }
    INT g_iHandZTheshold = (INT)( 110.0f * pRefinementData->m_fHeadToSPine / 0.6f );
    g_iHandZTheshold = min( g_iHandZTheshold, 110 );
    INT iTheshold = iTopThreeSmallest[0] + g_iHandZTheshold;
    if ( iTopThreeSmallest[2] != 10000 )
    {
        iTheshold = iTopThreeSmallest[2] + g_iHandZTheshold;
    }
    else if ( iTopThreeSmallest[1] != 10000 )
    {
        iTheshold = iTopThreeSmallest[1] + g_iHandZTheshold;
    }
    
    for ( INT iY12x12 = YRange.iMin; iY12x12 < YRange.iMax; ++iY12x12 )
    {
        for ( INT iX12x12 = XRange.iMin; iX12x12 < XRange.iMax; ++iX12x12 )
        {             
            if ( pTSD->bValid80x60Voxels[iY12x12][iX12x12] )
            {
                INT iX80x60 = iX12x12 - iNumSearchPixels + iRefinedPosition80x60.iX;
                INT iY80x60 = iY12x12 - iNumSearchPixels + iRefinedPosition80x60.iY;

                INT iX320x240 = iX80x60 * 4;
                INT iY320x240 = iY80x60 * 4;

                for ( INT iY4x4 = 0; iY4x4 < 4; ++iY4x4 )
                {
                    INT iFinalY = iY320x240 + iY4x4;
                    for ( INT iX4x4 = 0; iX4x4 < 4; ++iX4x4 )
                    {
                        INT iFinalX = iX320x240 + iX4x4;  

                        INT iDist = ( iFinalX - iSearchLocation320x240.iX ) * ( iFinalX - iSearchLocation320x240.iX ) + 
                            ( iFinalY - iSearchLocation320x240.iY ) * ( iFinalY - iSearchLocation320x240.iY );

                        if ( iDist < pTSD->iSquaredSizeOfHandTheshold )
                        {
                            USHORT uValue = pDepth320x240[ iFinalY * iShortPitch320x240 + iFinalX ];
                            USHORT uDepth = uValue >> 3;
                            USHORT uTexelPlayerIndex  = uValue & 0x0007;
                            if ( uDepth < iTheshold && uPlayerIndex == uTexelPlayerIndex )
                            {
                                iAverageX += iFinalX;
                                iAverageY += iFinalY;
                                ++iTotalCount;
                                iAverageZ += (INT)uDepth;
#ifdef _DEBUG
                                pHandSpecificData->m_VisualizeHandFramesData.m_FrameData[iY12x12*4+iY4x4][iX12x12*4+iX4x4] = 1;    
#endif
                            }
#ifdef _DEBUG
                            else 
                            {
                                pHandSpecificData->m_VisualizeHandFramesData.m_FrameData[iY12x12*4+iY4x4][iX12x12*4+iX4x4] = 0;    
                            }
#endif
                        }
                        
                    }
                }



            }
        }
    }


    if ( iTotalCount == 0 )
    {
        *pRefinedHand = pHandSpecificData->m_vRawHand;
    }
    else 
    {
        TransformScreenToWorld320x240( (FLOAT)iAverageX / (FLOAT)iTotalCount, 
                            (FLOAT)iAverageY / (FLOAT)iTotalCount, 
                           (FLOAT)iAverageZ / (FLOAT)(1000.0*(FLOAT)iTotalCount ),
                            pRefinedHand
                            );
    }
    return iTotalCount;
}

//--------------------------------------------------------------------------------------
// Name: LowResOctagonAverageSearch
// Desc: This function calcuates an avareage 80x60 texel value by averaging the 3x3 grid
// in all 8 directions.
//--------------------------------------------------------------------------------------
VOID LowResOctagonAverageSearch( OctagonAverageSearch* pOct80x60, 
                                 USHORT* pDepth80x60, 
                                 INT iShortPitch80x60, 
                                 INT3Vector SearchLocation, 
                                 USHORT uPlayerIndex,
                                 INT iRadius )
{
        
    INT3Vector CurrentSearch = SearchLocation;
    for ( INT iDirection = 0; iDirection < 8; iDirection++ )
    {
        CurrentSearch.iX = g_SpiralSearchOffsets[iDirection][0] * iRadius + SearchLocation.iX;
        CurrentSearch.iY = g_SpiralSearchOffsets[iDirection][1] * iRadius + SearchLocation.iY;

        Clamp80x60( &CurrentSearch.iX, &CurrentSearch.iY );

        pOct80x60->iValuesInAverage[iDirection] =
            Get3x3AverageOnArm80x60( pDepth80x60, iShortPitch80x60, CurrentSearch, 
                uPlayerIndex, g_iRemoveLargeOutlierThreshold, g_iThresholdToSearchPastTheHand,
                &pOct80x60->iCellAverageDepth[iDirection] );
    }
       
}

//--------------------------------------------------------------------------------------
// Name: CalculateSearchDirections
// Desc: This function calcuates the 5 potential search directions for the vector from the 
// elbow to the hand
//--------------------------------------------------------------------------------------
VOID CalculateSearchDirections( INT iHandX, INT iHandY, INT iElbowX, INT iElbowY, INT* p5Search )
{
    static CONST FLOAT XM_PIDIV8 =  XM_PI / 8.0f;

    FLOAT fAngle = 0.0f;
    FLOAT fDirection[] = { (FLOAT)( iHandX - iElbowX  ), 
        (FLOAT)( iHandY - iElbowY ) };

    fAngle = atan2f( fDirection[1], fDirection[0] );
    fAngle += XM_PI + XM_PIDIV8;
    fAngle /= XM_PI;
    fAngle *= 4.0f;
    fAngle = fAngle;
    INT iAngle = (INT)fAngle;
    iAngle %= 8;
    iAngle = iAngle;

    p5Search[SEARCH_FORWARD] = (iAngle );
    p5Search[SEARCH_DIAGONAL_LEFT] = (iAngle + 7) % 8;
    p5Search[SEARCH_DIAGONAL_RIGHT] = (iAngle + 1) % 8;
    p5Search[SEARCH_ORTHOGONAL_LEFT] = (iAngle + 6) % 8;
    p5Search[SEARCH_ORTHOGONAL_RIGHT] = (iAngle + 2) % 8;

}

//--------------------------------------------------------------------------------------
// Name: RefineWith320x240
// Desc: Take the 80x60 coordinate to the 320x240 map for refinement
//--------------------------------------------------------------------------------------
BOOL RefineWith320x240( USHORT* pDepth320x240, 
                        INT iShortPitch320x240,
                        USHORT* pDepth80x60,
                        INT iShortPitch80x60,
                        RefinementData* pRefinementData,
                        HandSpecificData* pHandSpecificData,
                        TemporaryStackData* pTSD
                      )
{
    XMVECTOR vRefinedHand;
    INT iPixelsReturned = 0;

    if ( !pHandSpecificData->m_bRevertedToRawData ) 
    {       
        iPixelsReturned = FindAverageForHand320x240( pDepth320x240, 
            iShortPitch320x240, 
            pDepth80x60,
            iShortPitch80x60,
            pRefinementData,
            pHandSpecificData,
            &vRefinedHand,
            pTSD
        );
    }
    else 
    {
        vRefinedHand = pHandSpecificData->m_vRawHand;
    }

    XMVECTOR vElbowHand = pHandSpecificData->m_vRawElbow - vRefinedHand;
    XMFLOAT4 fElbowHand;
    XMStoreFloat4( &fElbowHand, vElbowHand );
    FLOAT fRefinedForearmLength = XMVectorGetX( XMVector4Length( vElbowHand ) );
    FLOAT fTolerence = 1.5f;
    INT iMaxJump = 200;
    if ( fabsf (fElbowHand.x ) >  fabsf ( fElbowHand.z ) || fabsf( fElbowHand.y ) > fabsf ( fElbowHand.z ) )
    {
        fTolerence = 1.3f;
        iMaxJump = 100;
    }

    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX < 80 );
    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY < 60 );

    pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iZ = 
        pDepth80x60[pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY * iShortPitch80x60] >> 3;
    if ( ( fRefinedForearmLength / pHandSpecificData->m_fForearmLength ) > fTolerence ) 
    {
        vRefinedHand = pHandSpecificData->m_vRawHand;
        pHandSpecificData->m_bRevertedToRawData = TRUE;
    }
    else if ( abs( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iZ - 
        (INT)(XMVectorGetZ( vRefinedHand ) * 1000.0f ) ) > iMaxJump )
    {
        vRefinedHand = pHandSpecificData->m_vRawHand;
        pHandSpecificData->m_bRevertedToRawData = TRUE;
    }

    XMVECTOR vPreviousRefinedHand = pHandSpecificData->m_vRefinedHand;

    FLOAT fTetheringDistanceLimit = 0.1f;
    
    XMVECTOR tetherVector = vRefinedHand - vPreviousRefinedHand;
    FLOAT interpolationValue = min( 1,XMVector3Length(tetherVector).x/fTetheringDistanceLimit );
    vRefinedHand = vPreviousRefinedHand + tetherVector * interpolationValue;
    
    if ( pHandSpecificData->m_bRevertedToRawDataPreviousFrame != pHandSpecificData->m_bRevertedToRawData )
    {
        pHandSpecificData->m_vOffsetRawToRefined =  pHandSpecificData->m_vRefinedHand - vRefinedHand;
    }

    pHandSpecificData->m_vOffsetRawToRefined *= 0.8f;
    pHandSpecificData->m_vRefinedHand = vRefinedHand + pHandSpecificData->m_vOffsetRawToRefined;
    pHandSpecificData->m_bRevertedToRawDataPreviousFrame = pHandSpecificData->m_bRevertedToRawData;

    FLOAT fForearmLengthUpdate = XMVectorGetX( XMVector4Length( pHandSpecificData->m_vRawElbow - vRefinedHand ) );
    pHandSpecificData->m_fForearmLength = 0.3f * fForearmLengthUpdate + 0.7f * pHandSpecificData->m_fForearmLength;       

    return TRUE;

}


//--------------------------------------------------------------------------------------
// Name: ClimbOntoHand80x60
// Desc: Climb onto the highest close point.  This will help climb onto the arm if the 
// skeleton is off the depth map.
//--------------------------------------------------------------------------------------
VOID ClimbOntoHand80x60 ( USHORT* pDepth80x60, INT iShortPitch80x60, RefinementData* pRefinementData, HandSpecificData* pHandSpecificData )
{

    XMFLOAT4 fHandElbowDiff;


    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX < 80 );
    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY < 60 );

    OctagonAverageSearch OctagonAverageSearch80x60;
    INT i80x60RawDepth = pDepth80x60[ pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + 
        pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY * iShortPitch80x60 ] 
        >> 3;


    // when ST puts the arm out in space it's not on the depth map. so we're looking for the arm to be closer than the raw depth map position.
    if ( i80x60RawDepth - pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iZ < 20 && i80x60RawDepth != 0 ) 
    {
        return;
    }

    XMVECTOR vScreenHand;
    TransformScreenToWorld80x60( (FLOAT)pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, 
        (FLOAT)pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY,
        (FLOAT)i80x60RawDepth * 0.001f, &vScreenHand ); 
    vScreenHand -= pHandSpecificData->m_vRawElbow;
    XMStoreFloat4( &fHandElbowDiff, vScreenHand );

    // we sample the depth map rather than relying on the Skeleton's depth position.  
    FLOAT fSquaredDistanceHandToElbowWorld = fHandElbowDiff.x * fHandElbowDiff.x + 
        fHandElbowDiff.y * fHandElbowDiff.y + 
        fHandElbowDiff.z * fHandElbowDiff.z;  


    memset( &OctagonAverageSearch80x60, 0, sizeof( OctagonAverageSearch ) );
    LowResOctagonAverageSearch( &OctagonAverageSearch80x60, pDepth80x60, iShortPitch80x60, 
        pHandSpecificData->INT3ScreenSpaceHandRefined80x60,pRefinementData->m_uSegmentationIndex, 
        g_nWalkOntoHandSearchRadius + 1 );
    
    // hand is closer to camera
    INT iSmallest = i80x60RawDepth; 
    if ( iSmallest == 0 ) iSmallest = MAX_REAL_DEPTH;
    INT iSmallestIndex = -1;
    for ( int iIndex = 0; iIndex < 8; ++iIndex )
    {
        if ( OctagonAverageSearch80x60.iValuesInAverage[iIndex] > 2 
            && OctagonAverageSearch80x60.iCellAverageDepth[iIndex].iZ <  iSmallest )
        {

            INT3Vector iNewRefinedPosition = OctagonAverageSearch80x60.iCellAverageDepth[iIndex];

            XMVECTOR vScreenHand;
            TransformScreenToWorld80x60( (FLOAT)iNewRefinedPosition.iX, 
                (FLOAT)iNewRefinedPosition.iY, 
                (FLOAT)iNewRefinedPosition.iZ * 0.001f, &vScreenHand ); 
            vScreenHand -= pHandSpecificData->m_vRawElbow;
            XMStoreFloat4( &fHandElbowDiff, vScreenHand );

            // we sample the depth map rather than relying on the Skeleton's depth position.  
            FLOAT fSquaredDistanceNewHand = fHandElbowDiff.x * fHandElbowDiff.x + 
                fHandElbowDiff.y * fHandElbowDiff.y + 
                fHandElbowDiff.z * fHandElbowDiff.z;  

            if ( fSquaredDistanceNewHand / fSquaredDistanceHandToElbowWorld  > 0.75f || fSquaredDistanceHandToElbowWorld > 
                (pRefinementData->m_fHeadToSPine * pRefinementData->m_fHeadToSPine * 0.3f)  )
            {

                iSmallest = OctagonAverageSearch80x60.iCellAverageDepth[iIndex].iZ;
                iSmallestIndex = iIndex;
            }
        }
    }
    if ( iSmallestIndex != -1 )
    {
        pHandSpecificData->INT3ScreenSpaceHandRefined80x60 = OctagonAverageSearch80x60.iCellAverageDepth[iSmallestIndex];
        Clamp80x60( &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY );
    }

    for ( INT iOneStepSearch = 0; iOneStepSearch < 3; ++iOneStepSearch )
    {
        assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX < 80 );
        assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY < 60 );

        INT i80x60RawDepth = pDepth80x60[ pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + 
            pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY * iShortPitch80x60 ] 
            >> 3;

        memset( &OctagonAverageSearch80x60, 0, sizeof( OctagonAverageSearch ) );
        LowResOctagonAverageSearch( &OctagonAverageSearch80x60, pDepth80x60, iShortPitch80x60, 
            pHandSpecificData->INT3ScreenSpaceHandRefined80x60,pRefinementData->m_uSegmentationIndex, 
            g_nWalkOntoHandSearchRadius );
        
        // hand is closer to camera
        INT iSmallest = i80x60RawDepth; 
        if ( iSmallest == 0 ) iSmallest = MAX_REAL_DEPTH;
        INT iSmallestIndex = -1;
        for ( int iIndex = 0; iIndex < 8; ++iIndex )
        {
            if ( OctagonAverageSearch80x60.iValuesInAverage[iIndex] > 2 
                && OctagonAverageSearch80x60.iCellAverageDepth[iIndex].iZ <  iSmallest )
            {
                iSmallest = OctagonAverageSearch80x60.iCellAverageDepth[iIndex].iZ;
                iSmallestIndex = iIndex;
            }
        }
        if ( iSmallestIndex != -1 )
        {
            INT3Vector iNewRefinedPosition = OctagonAverageSearch80x60.iCellAverageDepth[iSmallestIndex];

            XMVECTOR vScreenHand;
            TransformScreenToWorld80x60( (FLOAT)iNewRefinedPosition.iX, 
                (FLOAT)iNewRefinedPosition.iY, 
                (FLOAT)iNewRefinedPosition.iZ * 0.001f, &vScreenHand ); 
            vScreenHand -= pHandSpecificData->m_vRawElbow;
            XMStoreFloat4( &fHandElbowDiff, vScreenHand );

            // we sample the depth map rather than relying on the Skeleton's depth position.  
            FLOAT fSquaredDistanceNewHand = fHandElbowDiff.x * fHandElbowDiff.x + 
                fHandElbowDiff.y * fHandElbowDiff.y + 
                fHandElbowDiff.z * fHandElbowDiff.z;  

            if ( fSquaredDistanceNewHand / fSquaredDistanceHandToElbowWorld  > 0.75f || fSquaredDistanceHandToElbowWorld > 
                (pRefinementData->m_fHeadToSPine * pRefinementData->m_fHeadToSPine * 0.3f)  )
            {
                pHandSpecificData->INT3ScreenSpaceHandRefined80x60 = iNewRefinedPosition;
                Clamp80x60( &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY );
            }
        }
        else
        {
            break;
        }
    }


#ifdef _DEBUG
    pHandSpecificData->INT3ScreenSpaceHandJumpOnArm80x60 = pHandSpecificData->INT3ScreenSpaceHandRefined80x60;
#endif
}



//--------------------------------------------------------------------------------------
// Name: DirectionOnEdge
// Desc: Test the direction and the 2 adjancent directions against a threshold
// to determine if we're on an edge.
//--------------------------------------------------------------------------------------
BOOL DirectionOnEdge( USHORT* pDepth80x60, INT iShortPitch80x60, INT3Vector EdgeWalkLocation, SHORT uPlayerIndex, INT iDirection, INT iComparisonValue )
{

    INT iGoodEdges = 0;


    INT2Vector PastEdgeTestCoord;
    PastEdgeTestCoord.iX = EdgeWalkLocation.iX;
    PastEdgeTestCoord.iY = EdgeWalkLocation.iY;
    if ( PastEdgeTestCoord.iX >= 0 && PastEdgeTestCoord.iX < 80 && PastEdgeTestCoord.iY >=0 && PastEdgeTestCoord.iY < 60 )
    {
        assert( PastEdgeTestCoord.iX < 80 );
        assert( PastEdgeTestCoord.iY < 60 );

        INT iValue = pDepth80x60[ PastEdgeTestCoord.iY * iShortPitch80x60 + PastEdgeTestCoord.iX ];
        INT iDepth = iValue >> 3;
        USHORT uIndex = (USHORT)iValue & 0x0007;
        if ( ( iDepth - iComparisonValue ) < g_iWalktoEndofHandTheshold && uIndex == uPlayerIndex )
        {
            ++iGoodEdges;
        }
    }

    if ( iGoodEdges > 0 )
        return FALSE;
    else
        return TRUE;

}


//--------------------------------------------------------------------------------------
// Name: WalkToEndofHand80x60
// Desc: Follow the current position away from the elbow towards the end of the hand
// elbow to the hand
//--------------------------------------------------------------------------------------
VOID WalkToEndofHand80x60 ( USHORT* pDepth80x60, INT iShortPitch80x60, RefinementData* pRefinementData, HandSpecificData* pHandSpecificData )
{
    INT iDirections[SEARCH_DIRECTION_COUNT];
    CalculateSearchDirections( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, 
        pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY, 
        pHandSpecificData->INT3ScreenSpaceElbow80x60.iX, 
        pHandSpecificData->INT3ScreenSpaceElbow80x60.iY, iDirections );

    INT iAverageForDirection[SEARCH_DIRECTION_COUNT];
    INT nPixelsUsedinAverageForDirection[SEARCH_DIRECTION_COUNT];
   
    INT iSmallestValue = MAX_REAL_DEPTH;
    INT iZforSmallest = -1;
    INT iOffset[] = {0,0};

    INT3Vector WalkLocation = pHandSpecificData->INT3ScreenSpaceHandRefined80x60; 
    INT iCurrentMarchLocationDepth = 0;
    INT iComparison = 0;
    INT iLastSmallestIndex = -1;
    INT iSmallestTest = -1;

    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX < 80 );
    assert( pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY < 60 );

    INT i80x60RawDepth = pDepth80x60[ pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + 
        pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY * iShortPitch80x60 ] >> 3;

    XMVECTOR vScreenHand;
    TransformScreenToWorld80x60( (FLOAT)pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, 
        (FLOAT)pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY,
        (FLOAT)i80x60RawDepth * 0.001f, &vScreenHand ); 
    vScreenHand -= pHandSpecificData->m_vRawElbow;
    FLOAT fHandToElbowLength = XMVectorGetX( XMVector4Length( vScreenHand ) );
    FLOAT fBodyRelativeArmLength =  fHandToElbowLength/ pRefinementData->m_fHeadToSPine;
    INT nPlayerSpecificIterations =g_nWalkToEdgeIterations;
    if ( fBodyRelativeArmLength < 0.5f )
    {
        nPlayerSpecificIterations +=3;
    }

    for ( INT iWalkIndex = 0; iWalkIndex < nPlayerSpecificIterations; ++iWalkIndex )
    {   
        WalkLocation.iX = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + iOffset[0];
        WalkLocation.iY = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY + iOffset[1];

        Clamp80x60( &WalkLocation.iX, &WalkLocation.iY );
        
        assert( WalkLocation.iX < 80 );
        assert( WalkLocation.iY < 60 );

        iCurrentMarchLocationDepth = pDepth80x60[ WalkLocation.iX + WalkLocation.iY * iShortPitch80x60] >> 3;
        if ( abs( iCurrentMarchLocationDepth - pHandSpecificData->INT3ScreenSpaceHandRaw80x60.iZ ) > g_iAcceptableRawDepthMapDifference )
        {   
            // We've fallen off the hand.
            break;
        }

        iComparison = iCurrentMarchLocationDepth;
        INT3Vector EdgeWalkLocation = WalkLocation;
        EdgeWalkLocation.iZ = iCurrentMarchLocationDepth;
        // Find the average for the potential search directions
        for ( INT iIndex = 0; iIndex < SEARCH_DIRECTION_COUNT; ++iIndex )
        {
            EdgeWalkLocation.iX = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + 
                iOffset[0] + g_SpiralSearchOffsets[iDirections[iIndex]][0];
            EdgeWalkLocation.iY = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY + 
                iOffset[1] + g_SpiralSearchOffsets[iDirections[iIndex]][1];

            Clamp80x60( &EdgeWalkLocation.iX, &EdgeWalkLocation.iY );

            assert( EdgeWalkLocation.iX < 80 );
            assert( EdgeWalkLocation.iY < 60 );

            USHORT iValue = pDepth80x60[ EdgeWalkLocation.iX + EdgeWalkLocation.iY * iShortPitch80x60 ]; 
            INT iDepth = iValue >> 3;
            USHORT iPlayerIndex = iValue & 0x0007;
            if ( iPlayerIndex != pRefinementData->m_uSegmentationIndex )
            {
                nPixelsUsedinAverageForDirection[iIndex] = 0;
                iAverageForDirection[iIndex] = 10000;
            }
            else 
            {
                nPixelsUsedinAverageForDirection[iIndex] = 9;
                iAverageForDirection[iIndex] = iDepth;
            }

            //nPixelsUsedinAverageForDirection[iIndex] = Get3x3Average80x60WithPlayerIndex( pDepth80x60, iShortPitch80x60, EdgeWalkLocation,
            //    pRefinementData->m_uSegmentationIndex, &iAverageForDirection[iIndex] );

            // We're following the average if it's within a threshold.  
            //We also check to make sure we don't follow the averages that lead up to the edge!
            if ( DirectionOnEdge( pDepth80x60, iShortPitch80x60, EdgeWalkLocation, pRefinementData->m_uSegmentationIndex, iDirections[iIndex], iComparison ) )
            {
                iAverageForDirection[iIndex] = 10000;    
            }

        }

        // find the correct direction to walk
        
        iSmallestTest = -1;
        iSmallestValue = MAX_REAL_DEPTH;
        iZforSmallest = -1;
        INT iOffLimitsDirection = -1;
        if ( iLastSmallestIndex == SEARCH_ORTHOGONAL_LEFT )
        {
            iOffLimitsDirection = SEARCH_ORTHOGONAL_RIGHT;
        }
        else if ( iLastSmallestIndex == SEARCH_ORTHOGONAL_RIGHT )
        {
            iOffLimitsDirection = SEARCH_ORTHOGONAL_LEFT;
        }
        
        for ( INT iDirectionsIndex = 0; iDirectionsIndex < SEARCH_DIRECTION_COUNT; ++iDirectionsIndex )
        {
            INT Test = abs( iAverageForDirection[iDirectionsIndex] - iComparison);
            if ( iSmallestValue > Test && iDirectionsIndex != iOffLimitsDirection )
            {
                iSmallestTest = iDirectionsIndex;
                iSmallestValue = Test;
            }
        }
//////////////////////////
        // If we walk left and right without encountering an edge, then we've walked onto another body part and we need to stop.

        INT iRightWalkIterations = 0;
        



        INT iEdgeDelta[] = { 120, 120, 120, 120, 120};
        
        for ( INT iEdgeDeltaIndex = 0; iEdgeDeltaIndex < SEARCH_DIRECTION_COUNT; ++iEdgeDeltaIndex )
        {
            EdgeWalkLocation.iX = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX + 
                iOffset[0];
            EdgeWalkLocation.iY = pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY + 
                iOffset[1];
            for ( iRightWalkIterations = 0; 
                iRightWalkIterations < 10 && EdgeWalkLocation.iX < 80 && EdgeWalkLocation.iX >= 0 && EdgeWalkLocation.iY < 60 && EdgeWalkLocation.iY >=0; 
                ++iRightWalkIterations)
            {
                EdgeWalkLocation.iX += g_SpiralSearchOffsets[iDirections[iEdgeDeltaIndex]][0];
                EdgeWalkLocation.iY += g_SpiralSearchOffsets[iDirections[iEdgeDeltaIndex]][1];

                Clamp80x60( &EdgeWalkLocation.iX, &EdgeWalkLocation.iY );

                assert( EdgeWalkLocation.iX < 80 );
                assert( EdgeWalkLocation.iY < 60 );

                USHORT uValue = pDepth80x60[ iShortPitch80x60 * EdgeWalkLocation.iY + EdgeWalkLocation.iX];
                INT iDepth = uValue >> 3;

                //INT iDepth = 0;
                //Get3x3Average80x60( pDepth80x60, iShortPitch80x60, EdgeWalkLocation, &iDepth );

                if ( iDepth - iComparison > 30 )// || ( uValue & 0x0007 ) != pRefinementData->m_uSegmentationIndex )
                {
                    iEdgeDelta[iEdgeDeltaIndex] =iDepth - iComparison;
                    break;
                }
            }
        }




//////////////////////////


        INT iEdgeWalkBasedTheshold = iEdgeDelta[0];
        for ( INT iIndex = 1; iIndex < SEARCH_DIRECTION_COUNT; ++ iIndex )
        {
            iEdgeWalkBasedTheshold = min ( iEdgeWalkBasedTheshold, iEdgeDelta[iIndex] );
        }
        

        iLastSmallestIndex = iSmallestTest;
        if ( iSmallestValue > iEdgeWalkBasedTheshold || iSmallestTest == -1 )
        {
            break;
        }
        else 
        {
            iOffset[0] += g_SpiralSearchOffsets[iDirections[iSmallestTest]][0]; 
            iOffset[1] += g_SpiralSearchOffsets[iDirections[iSmallestTest]][1]; 
        }


    }

    pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX += iOffset[0];// / 2;
    pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY += iOffset[1];// / 2;
    Clamp80x60( &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX, &pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY );

#ifdef _DEBUG
    pHandSpecificData->INT3ScreenSpaceHandWalkToEndofArm80x60 = pHandSpecificData->INT3ScreenSpaceHandRefined80x60;
#endif

}

VOID InitializeHand( NUI_SKELETON_POSITION_INDEX eHand, 
                    NUI_SKELETON_POSITION_INDEX eElbow,
                    RefinementData* pRefinementData,
                    const XMVECTOR pSkeletonJoints[NUI_SKELETON_POSITION_COUNT], 
                    HandSpecificData* pHandData,
                    TemporaryStackData* pTSD )
{
#ifdef _DEBUG
    memset( &pHandData->INT3ScreenSpaceHandJumpOnArm80x60, 0, sizeof(INT3Vector) );    
    memset( &pHandData->INT3ScreenSpaceHandWalkToEndofArm80x60, 0, sizeof(INT3Vector) );    
    memset( &pHandData->INT3ScreenSpaceHandCenterAroundEndofArm80x60, 0, sizeof(INT3Vector) );    

#endif
    

    FLOAT fSearchRadiusAtNearPlane =( pRefinementData->m_fHeadToSPine * 20.0f );
    
    FLOAT fRadius = fSearchRadiusAtNearPlane / XMVectorGetZ( pSkeletonJoints[eHand] );
    INT iSize =  sizeof( TemporaryStackData );
    memset( pTSD, 0, iSize );
    //fRadius += 0.5f; // take into account rounding;

    pTSD->iVoxelSearchRadius = min( (INT)fRadius, 6 );
    
    fRadius *= 4.0f;
    fRadius = min( fRadius, 24.0f );

    pTSD->iSquaredSizeOfHandTheshold = (INT) ( fRadius * fRadius );

    pHandData->m_vRawElbow = pSkeletonJoints[eElbow];
    pHandData->m_vRawHand = pSkeletonJoints[eHand];

    TransformWorldToScreen80x60( pHandData->m_vRawElbow, 
        &pHandData->INT3ScreenSpaceElbow80x60 );
    
    TransformWorldToScreen80x60( pSkeletonJoints[eHand], 
        &pHandData->INT3ScreenSpaceHandRaw80x60 );
    ATG::INT3Vector* ScreenHand = &pHandData->INT3ScreenSpaceHandRaw80x60;
    if ( ScreenHand->iX < 0 ||  ScreenHand->iX > 79 || ScreenHand->iY < 0|| ScreenHand->iY > 59 )
    {
        pHandData->m_bRevertedToRawData = TRUE;
    }
    else
    {
        pHandData->m_bRevertedToRawData = FALSE;
    }
    pHandData->INT3ScreenSpaceHandRefined80x60  = pHandData->INT3ScreenSpaceHandRaw80x60;
    Clamp80x60( &pHandData->INT3ScreenSpaceHandRefined80x60.iX, &pHandData->INT3ScreenSpaceHandRefined80x60.iY );
    pHandData->eHand = eHand;

    
    FLOAT fForearmLength = XMVectorGetX( XMVector4Length( pHandData->m_vRawElbow - pHandData->m_vRawHand ) );
    if ( pHandData->m_fForearmLength == 0.0f )
    {
        pHandData->m_fForearmLength = fForearmLength;       
    }
}

VOID TurnOnVoxelMaskIfAnyValidNeighbor( USHORT* pDepth80x60, INT iShortPitch80x60, USHORT uPlayerIndex,
                                       INT2Vector iVoxelPos, INT2Vector iImagePos, TemporaryStackData* pTSD )
{
    if ( iImagePos.iX < 0 || iImagePos.iX > 79 || iImagePos.iY < 0 || iImagePos.iY > 59 ) return;
    if ( iVoxelPos.iX < 0 || iVoxelPos.iX > 11 || iVoxelPos.iY < 0 || iVoxelPos.iY > 11 ) return;

    assert( iImagePos.iX < 80 );
    assert( iImagePos.iY < 60 );

    INT iCurrentPixelDepth = pDepth80x60[iImagePos.iX + iImagePos.iY * iShortPitch80x60] >> 3;
    if ( iImagePos.iX > 0 && iVoxelPos.iX > 0)
    {
        if ( iImagePos.iY > 0 && iVoxelPos.iY > 0 ) 
        {  // upper left pixel
            if ( pTSD->bValid80x60Voxels[iVoxelPos.iY - 1][iVoxelPos.iX - 1] )
            {
                assert( ( iImagePos.iX - 1 ) < 80 );
                assert( ( iImagePos.iY - 1 ) < 60 );

                INT iValue = pDepth80x60[ ( iImagePos.iX - 1 ) + (iImagePos.iY - 1 ) * iShortPitch80x60 ];  
                INT iDepth = iValue >> 3;
                INT iIndex = iValue & 0x0007;
                if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
                {
                    pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                    return;
                }
            }
        }
        if ( pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX - 1] )
        { // left pixel
            assert( ( iImagePos.iX - 1 ) < 80 );
            assert( ( iImagePos.iY ) < 60 );

            INT iValue = pDepth80x60[ ( iImagePos.iX - 1 ) + (iImagePos.iY) * iShortPitch80x60 ];  
            INT iDepth = iValue >> 3;
            INT iIndex = iValue & 0x0007;
            if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
            {
                pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                return;
            }
        }
        if ( iImagePos.iY < 59 && iVoxelPos.iY < 11 )
        { //lower left pixel
            if ( pTSD->bValid80x60Voxels[iVoxelPos.iY + 1][iVoxelPos.iX - 1] )
            {
                assert( ( iImagePos.iX - 1 ) < 80 );
                assert( ( iImagePos.iY + 1 ) < 60 );

                INT iValue = pDepth80x60[ ( iImagePos.iX - 1 ) + (iImagePos.iY + 1) * iShortPitch80x60 ];  
                INT iDepth = iValue >> 3;
                INT iIndex = iValue & 0x0007;
                if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
                {
                    pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                    return;
                }
            }
        }
    }

    if ( iImagePos.iY > 0 && iVoxelPos.iY > 0 )
    {// top pixel
        if ( pTSD->bValid80x60Voxels[iVoxelPos.iY - 1][iVoxelPos.iX] )
        {
            assert( ( iImagePos.iX ) < 80 );
            assert( ( iImagePos.iY -1 ) < 60 );

            INT iValue = pDepth80x60[ ( iImagePos.iX) + (iImagePos.iY - 1 ) * iShortPitch80x60 ];  
            INT iDepth = iValue >> 3;
            INT iIndex = iValue & 0x0007;
            if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
            {
                pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                return;
            }
        }
    }
    if ( iImagePos.iY < 59  && iVoxelPos.iY < 11 )
    {// bottom pixel
        if ( pTSD->bValid80x60Voxels[iVoxelPos.iY + 1][iVoxelPos.iX] )
        {
            assert( ( iImagePos.iX ) < 80 );
            assert( ( iImagePos.iY + 1 ) < 60 );

            INT iValue = pDepth80x60[ ( iImagePos.iX) + (iImagePos.iY + 1) * iShortPitch80x60 ];  
            INT iDepth = iValue >> 3;
            INT iIndex = iValue & 0x0007;
            if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
            {
                pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                return;
            }
        }
    }
    if ( iImagePos.iX < 79 && iVoxelPos.iX < 11)
    {
        if ( iImagePos.iY > 0  && iVoxelPos.iY > 0 ) 
        {  // upper right pixel
            if ( pTSD->bValid80x60Voxels[iVoxelPos.iY - 1][iVoxelPos.iX + 1] )
            {
                assert( ( iImagePos.iX + 1 ) < 80 );
                assert( ( iImagePos.iY - 1 ) < 60 );

                INT iValue = pDepth80x60[ ( iImagePos.iX + 1 ) + (iImagePos.iY - 1 ) * iShortPitch80x60 ];  
                INT iDepth = iValue >> 3;
                INT iIndex = iValue & 0x0007;
                if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
                {
                    pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                    return;
                }
            }
        }
        if ( pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX + 1] )
        { // right pixel
            assert( ( iImagePos.iX + 1 ) < 80 );
            assert( ( iImagePos.iY ) < 60 );

            INT iValue = pDepth80x60[ ( iImagePos.iX + 1 ) + (iImagePos.iY) * iShortPitch80x60 ];  
            INT iDepth = iValue >> 3;
            INT iIndex = iValue & 0x0007;
            if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
            {
                pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                return;
            }
        }
        if ( iImagePos.iY < 59  && iVoxelPos.iY < 11 )
        { //lower right pixel
            if ( pTSD->bValid80x60Voxels[iVoxelPos.iY + 1][iVoxelPos.iX + 1] )
            {
                assert( ( iImagePos.iX + 1 ) < 80 );
                assert( ( iImagePos.iY + 1 ) < 60 );

                INT iValue = pDepth80x60[ ( iImagePos.iX + 1 ) + (iImagePos.iY + 1) * iShortPitch80x60 ];  
                INT iDepth = iValue >> 3;
                INT iIndex = iValue & 0x0007;
                if ( ( iCurrentPixelDepth - iDepth) < g_iBuildVoxelMaskTheshold && iIndex == uPlayerIndex )
                {
                    pTSD->bValid80x60Voxels[iVoxelPos.iY][iVoxelPos.iX] = TRUE;
                    return;
                }
            }
        }
    }

}


inline void FloodFillSingleLine( USHORT* pDepth80x60,
                                 INT& iShortPitch80x60,
                                 USHORT& uPlayerSegmentationIndex,
                                 INT2Vector& iMaskLoation, 
                                 INT2Vector& i80x60Location,
                                 INT2Vector& iDirection, 
                                 INT2Vector& iSearchDirection, 
                                 INT& iSearchIterations, 
                                 TemporaryStackData* pTSD )
{
    
    for ( INT iSearchIndex = 0; iSearchIndex < iSearchIterations; ++iSearchIndex )
    {
        assert( i80x60Location.iX < 80 );
        assert( i80x60Location.iY < 60 );

        USHORT uCurrentValue = pDepth80x60[ i80x60Location.iX + i80x60Location.iY * iShortPitch80x60 ];
        USHORT uCurrentSegmentationIndex = uCurrentValue & 0x0007;
        INT iCurrentDepth = uCurrentValue >> 3;
        
        if ( uCurrentSegmentationIndex == uPlayerSegmentationIndex )
        {
            INT2Vector iSearchMaskLocation = iMaskLoation;
            iSearchMaskLocation.iX -= iDirection.iX;
            iSearchMaskLocation.iY -= iDirection.iY;
            iSearchMaskLocation.iX += iSearchDirection.iX;
            iSearchMaskLocation.iY += iSearchDirection.iY;

            INT2Vector iSearch80x60Location = i80x60Location;
            iSearch80x60Location.iX -= iDirection.iX;
            iSearch80x60Location.iY -= iDirection.iY;
            iSearch80x60Location.iX += iSearchDirection.iX;
            iSearch80x60Location.iY += iSearchDirection.iY;
            
            assert( iSearch80x60Location.iX < 80 );
            assert( iSearch80x60Location.iY < 60 );

            INT iTempDepth = pDepth80x60[ iSearch80x60Location.iY * iShortPitch80x60 + iSearch80x60Location.iX ] >> 3;
            if ( pTSD->bValid80x60Voxels[iSearchMaskLocation.iY][iSearchMaskLocation.iX]
                && ( iCurrentDepth - iTempDepth ) < g_iBuildVoxelMaskTheshold )
            {
                pTSD->bValid80x60Voxels[iMaskLoation.iY][iMaskLoation.iX] = TRUE;// The voxel is flood filled  
            }

            iSearchMaskLocation.iX += iDirection.iX;
            iSearchMaskLocation.iY += iDirection.iY;
            iSearch80x60Location.iX += iDirection.iX;
            iSearch80x60Location.iY += iDirection.iY;

            assert( iSearch80x60Location.iX < 80 );
            assert( iSearch80x60Location.iY < 60 );

            iTempDepth = pDepth80x60[ iSearch80x60Location.iY * iShortPitch80x60 + iSearch80x60Location.iX ] >> 3;
            if ( pTSD->bValid80x60Voxels[iSearchMaskLocation.iY][iSearchMaskLocation.iX]
                && ( iCurrentDepth - iTempDepth ) < g_iBuildVoxelMaskTheshold )
            {
                pTSD->bValid80x60Voxels[iMaskLoation.iY][iMaskLoation.iX] = TRUE;// The voxel is flood filled  
            }

            iSearchMaskLocation.iX += iDirection.iX;
            iSearchMaskLocation.iY += iDirection.iY;
            iSearch80x60Location.iX += iDirection.iX;
            iSearch80x60Location.iY += iDirection.iY;

            assert( iSearch80x60Location.iX < 80 );
            assert( iSearch80x60Location.iY < 60 );

            iTempDepth = pDepth80x60[ iSearch80x60Location.iY * iShortPitch80x60 + iSearch80x60Location.iX ] >> 3;
            if ( pTSD->bValid80x60Voxels[iSearchMaskLocation.iY][iSearchMaskLocation.iX]
                && ( iCurrentDepth - iTempDepth ) < g_iBuildVoxelMaskTheshold )
            {
                pTSD->bValid80x60Voxels[iMaskLoation.iY][iMaskLoation.iX] = TRUE;// The voxel is flood filled  
            }
            
        }
        iMaskLoation.iX += iDirection.iX;
        iMaskLoation.iY += iDirection.iY;

        i80x60Location.iX += iDirection.iX;
        i80x60Location.iY += iDirection.iY;
    }
}



VOID BuildValidVoxelMask( USHORT* pDepth80x60, 
                          INT iShortPitch80x60,
                          RefinementData* pRefinementData,
                          HandSpecificData* pHandSpecificData,
                          TemporaryStackData* pTSD )
{
    INT3Vector Position80x60 = pHandSpecificData->INT3ScreenSpaceHandRefined80x60;
    
    // compute the inner 4 valid voxels.
    pTSD->bValid80x60Voxels[ 5 ][ 5 ] = TRUE;

    INT2Vector iVoxelPos;
    INT2Vector i80x60Pos;

    iVoxelPos.iX = 5;
    iVoxelPos.iY = 6;
    i80x60Pos.iX = Position80x60.iX;
    i80x60Pos.iY = Position80x60.iY + 1;
    TurnOnVoxelMaskIfAnyValidNeighbor( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex, iVoxelPos, i80x60Pos, pTSD );
    iVoxelPos.iX = 6;
    iVoxelPos.iY = 5;
    i80x60Pos.iX = Position80x60.iX + 1;
    i80x60Pos.iY = Position80x60.iY;
    TurnOnVoxelMaskIfAnyValidNeighbor( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex, iVoxelPos, i80x60Pos, pTSD );
    iVoxelPos.iX = 6;
    iVoxelPos.iY = 6;
    i80x60Pos.iX = Position80x60.iX + 1;
    i80x60Pos.iY = Position80x60.iY + 1;
    TurnOnVoxelMaskIfAnyValidNeighbor( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex, iVoxelPos, i80x60Pos, pTSD );

    for ( INT iRingLoop = 1; iRingLoop < pTSD->iVoxelSearchRadius; ++iRingLoop )
    {
        INT2Vector iMaskLoation;
        INT2Vector i80x60Location;
        INT2Vector iDirection;
        INT2Vector iSearchDirection; 
        INT iSearchIterations;

        // Search the top row right
        
        iSearchIterations = iRingLoop * 2 + 2; 
        iMaskLoation.iX = 5 - iRingLoop;
        iMaskLoation.iY = 5 - iRingLoop;
        i80x60Location.iX = Position80x60.iX - iRingLoop;
        i80x60Location.iY = Position80x60.iY - iRingLoop;
        if ( iRingLoop == 5)
        {
            // special case the last iteration so we don't index outside of the array
            iMaskLoation.iX ++;   
            i80x60Location.iX ++;
            iSearchIterations -=2;
        }
        iDirection.iX = 1;
        iDirection.iY = 0;
        iSearchDirection.iX = 0;
        iSearchDirection.iY = 1;
        // Do bounds checking so we can index directly.
        if ( i80x60Location.iX < 1 )
        {
            iMaskLoation.iX += ( -i80x60Location.iX + 1 );
            iSearchIterations -= ( -i80x60Location.iX + 1 );   
            i80x60Location.iX = 1;
        }
        if ( i80x60Location.iX + iSearchIterations > 78 )
        {
            iSearchIterations = 78 - i80x60Location.iX ;    
        }
        if ( i80x60Location.iY > 0 )
        {
            FloodFillSingleLine( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex,
                iMaskLoation, i80x60Location, iDirection, iSearchDirection, iSearchIterations, pTSD );
        }
        
        // search the bottom row right
        iSearchIterations = iRingLoop * 2 + 2; 
        iMaskLoation.iX = 5 - iRingLoop;
        iMaskLoation.iY = 5 + iRingLoop + 1;
        i80x60Location.iX = Position80x60.iX - iRingLoop;
        i80x60Location.iY = Position80x60.iY + iRingLoop + 1;
        if ( iRingLoop == 5)
        {
            // special case the last iteration so we don't index outside of the array
            iMaskLoation.iX ++;   
            i80x60Location.iX ++;
            iSearchIterations -=2;
        }
        iDirection.iX = 1;
        iDirection.iY = 0;
        iSearchDirection.iX = 0;
        iSearchDirection.iY = -1;
        // Do bounds checking so we can index directly.
        if ( i80x60Location.iX < 1 )
        {
            iMaskLoation.iX += ( -i80x60Location.iX + 1 );
            iSearchIterations -= ( -i80x60Location.iX + 1 );   
            i80x60Location.iX = 1;
        }
        if ( i80x60Location.iX + iSearchIterations > 78 )
        {
            iSearchIterations = 78 - i80x60Location.iX ;    
        }
        if ( i80x60Location.iY < 60 )
        {
            FloodFillSingleLine( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex,
                iMaskLoation, i80x60Location, iDirection, iSearchDirection, iSearchIterations, pTSD );
        }
        
        // search bottom up
        iSearchIterations = iRingLoop * 2 + 2; 
        iMaskLoation.iX = 5 - iRingLoop;
        iMaskLoation.iY = 5 + iRingLoop + 1;
        i80x60Location.iX = Position80x60.iX - iRingLoop;
        i80x60Location.iY = Position80x60.iY + iRingLoop + 1;
        if ( iRingLoop == 5)
        {
            // special case the last iteration so we don't index outside of the array
            iMaskLoation.iY --;   
            i80x60Location.iY --;
            iSearchIterations -=2;
        }
        iDirection.iX = 0;
        iDirection.iY = -1;
        iSearchDirection.iX = 1;
        iSearchDirection.iY = 0;
        // Do bounds checking so we can index directly.
        if ( i80x60Location.iY - iSearchIterations < 1 )
        {
            iSearchIterations = i80x60Location.iY - 1;
        }
        if ( i80x60Location.iY > 58 )
        {
            iMaskLoation.iY -= ( i80x60Location.iY - 58 );
            iSearchIterations -= ( i80x60Location.iY - 58 );   
            i80x60Location.iY = 58;
        }

        if ( i80x60Location.iX > 0 )
        {
            FloodFillSingleLine( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex,
                iMaskLoation, i80x60Location, iDirection, iSearchDirection, iSearchIterations, pTSD );
        }

        iSearchIterations = iRingLoop * 2 + 2; 
        iMaskLoation.iX = 5 + iRingLoop + 1;
        iMaskLoation.iY = 5 + iRingLoop + 1;
        i80x60Location.iX = Position80x60.iX + iRingLoop + 1;
        i80x60Location.iY = Position80x60.iY + iRingLoop + 1;
        if ( iRingLoop == 5)
        {
            // special case the last iteration so we don't index outside of the array
            iMaskLoation.iY --;   
            i80x60Location.iY --;
            iSearchIterations -=2;
        }
        iDirection.iX = 0;
        iDirection.iY = -1;
        iSearchDirection.iX = -1;
        iSearchDirection.iY = 0;
        // Do bounds checking so we can index directly.
        if ( i80x60Location.iY - iSearchIterations < 1 )
        {
            iSearchIterations = i80x60Location.iY - 1;
        }
        if ( i80x60Location.iY > 58 )
        {
            iMaskLoation.iY -= ( i80x60Location.iY - 58 );
            iSearchIterations -= ( i80x60Location.iY - 58 );   
            i80x60Location.iY = 58;
        }
        if ( i80x60Location.iX < 80 )
        {
            FloodFillSingleLine( pDepth80x60, iShortPitch80x60, pRefinementData->m_uSegmentationIndex,
                iMaskLoation, i80x60Location, iDirection, iSearchDirection, iSearchIterations, pTSD );
        }

    }

}

VOID BuildValidVoxelMaskAndCenter ( USHORT* pDepth80x60, 
                                    INT iShortPitch80x60,
                                    RefinementData* pRefinementData,
                                    HandSpecificData* pHandSpecificData,
                                    TemporaryStackData* pTSD )
{
    if ( pHandSpecificData->m_bRevertedToRawData ) return;
    INT iSize = sizeof(BOOL) * 12 * 12;
    memset( pTSD->bValid80x60Voxels, 0, iSize );
    BuildValidVoxelMask( pDepth80x60, iShortPitch80x60, pRefinementData, pHandSpecificData, pTSD );
}

//--------------------------------------------------------------------------------------
// Name: CenterPointOnHandAverageSearch
// Desc: Find the hand inthe 80x60 map
//--------------------------------------------------------------------------------------
VOID CenterPointOnHandAverageSearch ( USHORT* pDepth80x60, 
                                      INT iShortPitch80x60,
                                      RefinementData* pRefinementData,
                                      HandSpecificData* pHandSpecificData,
                                      TemporaryStackData* pTSD )
{
    ClimbOntoHand80x60( pDepth80x60, iShortPitch80x60, pRefinementData, pHandSpecificData );
    if ( pHandSpecificData->m_bRevertedToRawData )
    {
        return;
    }

    INT3Vector iElbow = pHandSpecificData->INT3ScreenSpaceElbow80x60;
    INT iElbowHandDistance = ( iElbow.iX - pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX ) *
                        ( iElbow.iX - pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iX ) +
                        ( iElbow.iY - pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY ) *
                        ( iElbow.iY - pHandSpecificData->INT3ScreenSpaceHandRefined80x60.iY );
    if ( iElbowHandDistance > 14 )
    {
        WalkToEndofHand80x60( pDepth80x60, iShortPitch80x60, pRefinementData, pHandSpecificData );
        if ( pHandSpecificData->m_bRevertedToRawData )
        {
            return;
        }
    }
    BuildValidVoxelMaskAndCenter( pDepth80x60, iShortPitch80x60, pRefinementData, pHandSpecificData, pTSD );
    
}

// We really need to test the tracking state of the joint, but we don't want to break the public API in
// approved libs, so simply check for (0,0,0,0)
inline BOOL IsJointValid( XMVECTOR vJoint )
{
    return XMVector4NotEqual( vJoint, XMVectorZero() );
}


HRESULT RefineHandsCore(   _In_     USHORT*                                pDepth80x60,
                        _In_     INT                                    iShortPitch80x60,
                        _In_     USHORT*                                pDepth320x240,
                        _In_     INT                                    iShortPitch320x240,
                        _In_     INT                                    iSegmentationIndex,
                        _In_     const XMVECTOR                         pSkeletonJoints[NUI_SKELETON_POSITION_COUNT],
                        _Inout_  RefinementData*                        pRefinementData )
{
    pRefinementData->m_uSegmentationIndex = (USHORT) iSegmentationIndex; 

    XMVECTOR vSpine = pSkeletonJoints[NUI_SKELETON_POSITION_SPINE];
    XMVECTOR vHead  = pSkeletonJoints[NUI_SKELETON_POSITION_HEAD];

    // Check if joints are valid
    if ( IsJointValid( vSpine ) &&
         IsJointValid( vHead ) )
    {
        XMVECTOR vHeadToSpine = vHead - vSpine;
        FLOAT fLength = XMVectorGetX( XMVector3Length( vHeadToSpine ) );

        if ( pRefinementData->m_fHeadToSPine == 0.0f )
        {
            pRefinementData->m_fHeadToSPine = fLength;
        }
        else 
        {
            pRefinementData->m_fHeadToSPine = 0.8f * pRefinementData->m_fHeadToSPine + 0.2f * fLength;
        }
    }
    
    // Check if right hand is valid
    XMVECTOR vHand  = pSkeletonJoints[ NUI_SKELETON_POSITION_HAND_RIGHT ];
    XMVECTOR vElbow = pSkeletonJoints[ NUI_SKELETON_POSITION_ELBOW_RIGHT ];
    if ( IsJointValid( vHand ) &&
         IsJointValid( vElbow ) )
    {
        TemporaryStackData TSDRight;       
        InitializeHand( NUI_SKELETON_POSITION_HAND_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, pRefinementData,
                        pSkeletonJoints, &pRefinementData->m_RightHandData, &TSDRight );

        PIXBeginNamedEvent( 0, "Refine80x60search Right" );
        CenterPointOnHandAverageSearch( pDepth80x60, iShortPitch80x60, pRefinementData, &pRefinementData->m_RightHandData, &TSDRight );
        PIXEndNamedEvent( );

        PIXBeginNamedEvent( 0, "Refine320x240search Right" );
            RefineWith320x240( pDepth320x240, iShortPitch320x240, pDepth80x60, iShortPitch80x60,
                pRefinementData, &pRefinementData->m_RightHandData, &TSDRight );
        PIXEndNamedEvent( );
    }
 
    // Check if left hand is valid
    vHand  = pSkeletonJoints[ NUI_SKELETON_POSITION_HAND_LEFT ];
    vElbow = pSkeletonJoints[ NUI_SKELETON_POSITION_ELBOW_LEFT ];
    if ( IsJointValid( vHand ) &&
         IsJointValid( vElbow ) )
    {
        TemporaryStackData TSDLeft;
        InitializeHand( NUI_SKELETON_POSITION_HAND_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, pRefinementData,
                        pSkeletonJoints, &pRefinementData->m_LeftHandData, &TSDLeft);

        PIXBeginNamedEvent( 0, "Refine80x60search Left" );
            CenterPointOnHandAverageSearch( pDepth80x60, iShortPitch80x60, pRefinementData, &pRefinementData->m_LeftHandData, &TSDLeft );
        PIXEndNamedEvent( );

        PIXBeginNamedEvent( 0, "Refine320x240search Left" );
            RefineWith320x240( pDepth320x240, iShortPitch320x240, pDepth80x60, iShortPitch80x60,
                pRefinementData, &pRefinementData->m_LeftHandData, &TSDLeft );
        PIXEndNamedEvent( );
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: RefineHands
// Desc: Use the 80x60 map to find the hand and then refine with the 320x240 map.
//--------------------------------------------------------------------------------------
HRESULT RefineHands (   _In_     LPDIRECT3DTEXTURE9                     pDepthAndSegmentationTexture320x240,
                        _In_     LPDIRECT3DTEXTURE9                     pDepthAndSegmentationTexture80x60,
                        _In_     INT                                    iSkeletonDataSlot,
                        _In_     const NUI_SKELETON_FRAME*              pSkeletonFrame,
                        _Inout_  RefinementData*                        pRefinementData,
                        _In_opt_ const D3DLOCKED_RECT*                  pDepthRect320x240,
                        _In_opt_ const D3DLOCKED_RECT*                  pDepthRect80x60
                 )
{
    if ( pDepthAndSegmentationTexture320x240 == NULL ||
         pDepthAndSegmentationTexture80x60 == NULL ||
         iSkeletonDataSlot < 0 ||
         iSkeletonDataSlot >= NUI_SKELETON_COUNT )
    {
        return E_INVALIDARG;
    }

    D3DLOCKED_RECT Rect320x240;
    D3DLOCKED_RECT Rect80x60;
    
    if ( pDepthRect320x240 == NULL )
    {
        pDepthAndSegmentationTexture320x240->LockRect( 0, &Rect320x240, 0, D3DLOCK_READONLY );
        pDepthRect320x240 = &Rect320x240;
    }
    if ( pDepthRect80x60 == NULL )
    {
        pDepthAndSegmentationTexture80x60->LockRect( 0, &Rect80x60, 0, D3DLOCK_READONLY );
        pDepthRect80x60 = &Rect80x60;
    }

    USHORT *pDepth80x60 = (USHORT *)pDepthRect80x60->pBits;
    INT iShortPitch80x60 = pDepthRect80x60->Pitch / sizeof(USHORT);
    USHORT *pDepth320x240 = (USHORT *)pDepthRect320x240->pBits;
    INT iShortPitch320x240 = pDepthRect320x240->Pitch / sizeof(USHORT);

    if ( iShortPitch80x60 < 0 ||
         iShortPitch320x240 < 0 ||
         pDepth80x60 == NULL ||
         pDepth320x240 == NULL )
    {
        pDepthAndSegmentationTexture320x240->UnlockRect( 0 );
        pDepthAndSegmentationTexture80x60->UnlockRect( 0 );

        return E_FAIL;
    }


    PIXBeginNamedEvent( 0, "Refine Both Hands Core" );
    RefineHandsCore( pDepth80x60,
        iShortPitch80x60,
        pDepth320x240,
        iShortPitch320x240,
        iSkeletonDataSlot + 1, // segmentation index is SkeletonData Slot + 1
        pSkeletonFrame->SkeletonData[iSkeletonDataSlot].SkeletonPositions,
        pRefinementData );
    PIXEndNamedEvent( );
        
    if ( pDepthRect320x240 != NULL )
    {
        pDepthAndSegmentationTexture320x240->UnlockRect( 0 );
    }
    if ( pDepthRect80x60 != NULL )
    {
        pDepthAndSegmentationTexture80x60->UnlockRect( 0 );
    }
    return S_OK;
}

// ********************************************************************************************************* //
// ********************************************************************************************************* //

}//namespace NuiHandRefinement

