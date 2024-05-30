// ********************************************************************************************************* //
// --------------------------------------------------------------------------------------------------------- //
//
// Copyright (c) Microsoft Corporation
//
// --------------------------------------------------------------------------------------------------------- //
// ********************************************************************************************************* //

#ifndef _ATG_NUI_HAND_REFINEMENT_H
#define _ATG_NUI_HAND_REFINEMENT_H


// ********************************************************************************************************* //
//
//  Namespace:
//

#include <nuiapi.h>

namespace ATG
{

static CONST INT g_iNuiRefinementHalfKernelSize320x240 = 24; // value * 2 must be a multiple of 8 for vmx code
static CONST INT g_iNuiRefinementHalfKernelSize80x60 = 6; // must be a multiple of g_iNuiRefinementHalfKernelSize320x240

struct INT3Vector
{
    INT iX, iY, iZ;
};

struct INT2Vector
{
    INT iX, iY;
};

struct INT2Range
{
    INT iMin, iMax;
};

#ifdef _DEBUG

class VisualizeHandFramesData {
public:
    CHAR m_FrameData[ g_iNuiRefinementHalfKernelSize320x240 * 2 ][ g_iNuiRefinementHalfKernelSize320x240 * 2];
};
#endif

struct HandSpecificData
{
    HandSpecificData() 
    {
        Reset();
    };
    ~HandSpecificData(){};


    // Call Reset when the state needs to be reset
    VOID Reset()
    {

        m_fForearmLength = 0.0f;
        m_vRawHand = XMVectorZero();
        m_vRefinedHand = XMVectorZero();
        m_vRawElbow = XMVectorZero();
        m_vOffsetRawToRefined = XMVectorZero();
        m_i320x240SearchRangeX.iMin = 0;
        m_i320x240SearchRangeX.iMax = 0;
        m_i320x240SearchRangeY.iMin = 0;
        m_i320x240SearchRangeY.iMax = 0;
        m_iHandSearchLocation320_240.iX = 0; 
        m_iHandSearchLocation320_240.iY = 0;
        m_i80x60SearchRangeX.iMin = 0;
        m_i80x60SearchRangeX.iMax = 0;
        m_i80x60SearchRangeY.iMin = 0;
        m_i80x60SearchRangeY.iMax = 0;
        INT3ScreenSpaceHandRaw80x60.iX = 0;
        INT3ScreenSpaceHandRaw80x60.iY = 0;
        INT3ScreenSpaceHandRaw80x60.iZ = 0;
        INT3ScreenSpaceHandRefined80x60.iX = 0;
        INT3ScreenSpaceHandRefined80x60.iY = 0;
        INT3ScreenSpaceHandRefined80x60.iZ = 0;
        INT3ScreenSpaceElbow80x60.iX = 0;
        INT3ScreenSpaceElbow80x60.iY = 0;
        INT3ScreenSpaceElbow80x60.iZ = 0;
        m_bRevertedToRawData = FALSE;
        m_bRevertedToRawDataPreviousFrame = FALSE;
        eHand = 0;    
        m_uMinFrom320x240 = 0;
#ifdef _DEBUG
        memset( &m_VisualizeHandFramesData, 0, sizeof( VisualizeHandFramesData ) );
#endif

    };
    
    XMVECTOR m_vRawHand;                                            // The hand that was received from the Raw skeleton.
    XMVECTOR m_vRefinedHand;                                        // The refined hand that will be returned to the user.
    XMVECTOR m_vRawElbow;                                           // The Elbow that correlates to the hand. i.e. right hand uses right elbow
    XMVECTOR m_vOffsetRawToRefined;                                 // An Offset that is set to interpolate between raw and refined data when we must fall back to raw data

    INT3Vector INT3ScreenSpaceHandRaw80x60;                         // The Raw hand in screen space. 
    INT3Vector INT3ScreenSpaceElbow80x60;                           // The Raw elbow in screen space.
    INT3Vector INT3ScreenSpaceHandRefined80x60;                     // The refined hand position in 80x60 space.
    INT eHand;                                                      // NUI_SKELETON_POSITION_INDEX

    INT2Range m_i320x240SearchRangeX;                               // The window in X used to search the 320x240 map for the final hand value.
    INT2Range m_i320x240SearchRangeY;                               // The window in Y used to search the 320x240 map for the final hand value.
    INT2Range m_i80x60SearchRangeX;                                 // The window in X used to search for the 80x60 center of the hand. 
    INT2Range m_i80x60SearchRangeY;                                 // The window in Y used to search for the 80x60 center of the hand.
                                                                    // The two values above are used to center the hand in the 80x60. This centered
                                                                    // Value is then used to center the 320x240 window for the final averageing.

    INT2Vector m_iHandSearchLocation320_240;                        // Location of window for the final search.

    BOOL m_bRevertedToRawData;                                      // Set this to true when we need to fall back to raw data
    BOOL m_bRevertedToRawDataPreviousFrame;                         // True when we fell back to raw data on the previous frame

    USHORT m_uMinFrom320x240;                                         // The value used to threshold the final average.     
    FLOAT m_fForearmLength;
#ifdef _DEBUG
    INT3Vector INT3ScreenSpaceHandJumpOnArm80x60;                   // The hand after jumping onto the arm.
    INT3Vector INT3ScreenSpaceHandWalkToEndofArm80x60;              // The hand after walking to the end of the arm.
    INT3Vector INT3ScreenSpaceHandCenterAroundEndofArm80x60;        // The hand after centering on the arm.
    VisualizeHandFramesData m_VisualizeHandFramesData;        
#endif

};



class RefinementData
{
public:
    RefinementData ()
    {
        Reset();
    };
    VOID Reset()
    {
        m_LeftHandData.Reset();
        m_RightHandData.Reset();
        m_uSegmentationIndex = 0;
        m_fHeadToSPine = 0.0f;
    }
    XMVECTOR GetRefinedRight()
    {
        return m_RightHandData.m_vRefinedHand;
    }
    XMVECTOR GetRefinedLeft()
    {
        return m_LeftHandData.m_vRefinedHand;
    }
    ~RefinementData(){};

    HandSpecificData m_LeftHandData;
    HandSpecificData m_RightHandData;
    FLOAT m_fHeadToSPine;


    USHORT m_uSegmentationIndex;


};

HRESULT RefineHands (  
                    _In_     LPDIRECT3DTEXTURE9                     pDepthAndSegmentationTexture320x240,
                    _In_     LPDIRECT3DTEXTURE9                     pDepthAndSegmentationTexture80x60,
                    _In_     INT                                    iSkeletonDataSlot,
                    _In_     const NUI_SKELETON_FRAME*              pSkeletonFrame,
                    _Inout_  RefinementData*                        pRefinementData,
                    _In_opt_ const D3DLOCKED_RECT*                  pDepthRect320x240,
                    _In_opt_ const D3DLOCKED_RECT*                  pDepthRect80x60
                 );


// ********************************************************************************************************* //
// ********************************************************************************************************* //

}//namespace NuiHandRefinement

// --------------------------------------------------------------------------------------------------------- //

#endif//__HANDREFINEMENTINTERNAL_H__

