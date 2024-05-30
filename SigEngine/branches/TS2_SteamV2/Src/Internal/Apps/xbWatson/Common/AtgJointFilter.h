//--------------------------------------------------------------------------------------
// JointFilter.h
//
// This file contains various filters for filtering Joints
//
// Microsoft Game Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


#pragma once
#ifndef _JOINT_FILTER_H
#define _JOINT_FILTER_H

#include <xtl.h>
#include <nuiapi.h>
#include <xnamath.h>

namespace ATG
{

    class FilterSkeletonData
    {
    public : 
        FilterSkeletonData(){};
        ~FilterSkeletonData(){};
        XMVECTOR m_Positions[NUI_SKELETON_POSITION_COUNT];

    };

    class FilterVelocitySingleEstimate
    {
    public:
        XMVECTOR m_vEstVel[NUI_SKELETON_POSITION_COUNT];
    };

    class FilterTaylorSeriesData
    {
      public:
        XMVECTOR vPos;
        XMVECTOR vEstVel;
        XMVECTOR vEstVel2;
        XMVECTOR vEstVel3;
    };

    class FilterTaylorSeries
    {

    public:
        FilterTaylorSeries( ) {Init();}

        // Amount of smoothing
        VOID Init( FLOAT fSmoothing = 0.5f )
        {
            m_fSmoothing = fSmoothing;
            Reset();
        };

        VOID Reset()
        {
            XMemSet( m_FilteredJoints, 0, sizeof( m_FilteredJoints )  );
            XMemSet( m_History, 0, sizeof( m_History ) );
        }

        VOID Update( const XMVECTOR* pJoints );

        XMVECTOR* GetFilteredJoints() {return &m_FilteredJoints[0];}

    private:
        XMVECTOR m_FilteredJoints[NUI_SKELETON_POSITION_COUNT];
        FilterTaylorSeriesData m_History[NUI_SKELETON_POSITION_COUNT];
        FLOAT m_fSmoothing;

    };

    class FilterBlendJoint 
    {
    public:
        FilterBlendJoint() : m_pSkeletonsData( NULL ), m_nFramesToAverage( 0 ), m_iCurrentFrame( 0 )
        {
            Init( 10 );
        };

        ~FilterBlendJoint() { delete[] m_pSkeletonsData; };

        VOID Init( INT nFramesToAverage )
        {
            m_nFramesToAverage = nFramesToAverage;
            m_pSkeletonsData = new FilterSkeletonData[ nFramesToAverage ];
            Reset();
        };

        VOID Reset()
        {
            memset( &m_FilteredSkeleton, 0, sizeof( FilterSkeletonData ) );
            memset( m_pSkeletonsData, 0, sizeof( FilterSkeletonData ) * m_nFramesToAverage );
            m_iCurrentFrame = 0;    
        }

        VOID Update( const XMVECTOR* pJoints );

        XMVECTOR* GetFilteredJoints( );    

    private:
        FilterSkeletonData* m_pSkeletonsData;
        FilterSkeletonData m_FilteredSkeleton;
        INT m_nFramesToAverage;
        INT m_iCurrentFrame;
    };

    class FilterVelDamp    
    {
    public:
        FilterVelDamp()
        {
            Init();
        };

        ~FilterVelDamp(){};

        VOID Init( )
        {
            Reset();
        }

        VOID Reset()
        {
            memset( m_SkeletonData, 0, sizeof(FilterSkeletonData) * 2 );
            memset( m_VelocityEstimate, 0, sizeof(FilterVelocitySingleEstimate) * 2 );
            m_iCurrentFrame = -1;
            m_iPreviousFrame = -1;    
        }

        VOID Update( const XMVECTOR* pJoints );

        XMVECTOR* GetFilteredJoints( );    

    private:
        FilterSkeletonData m_SkeletonData[2];
        FilterVelocitySingleEstimate m_VelocityEstimate[2];
        INT m_iCurrentFrame;
        INT m_iPreviousFrame;
    };


    class FilterDoubleExponentialData
    {
    public:
        XMVECTOR m_vRawPosition;
        XMVECTOR m_vFilteredPosition;
        XMVECTOR m_vTrend;
        DWORD    m_dwFrameCount;
    };

    class FilterDoubleExponential
    {
    public:
        FilterDoubleExponential() {Init();}

        VOID Init(FLOAT fSmoothing = 0.25f, FLOAT fCorrection = 0.25f, FLOAT fPrediction = 0.25f, FLOAT fJitterRadius = 0.03f, FLOAT fMaxDeviationRadius = 0.05f)
        {
            m_fMaxDeviationRadius = fMaxDeviationRadius; // Size of the max prediction radius Can snap back to noisy data when too high
            m_fSmoothing = fSmoothing;                   // How much smothing will occur.  Will lag when too high
            m_fCorrection = fCorrection;                 // How much to correct back from prediction.  Can make things springy
            m_fPrediction = fPrediction;                 // Amount of prediction into the future to use. Can over shoot when too high
            m_fJitterRadius = fJitterRadius;             // Size of the radius where jitter is removed. Can do too much smoothing when too high
            Reset();
        }

        VOID Reset()
        {
            XMemSet(m_FilteredJoints, 0, sizeof(m_FilteredJoints));
            XMemSet(m_History, 0, sizeof(m_History));
        }

        VOID Update( const NUI_SKELETON_DATA* pSkeletonData );

        XMVECTOR* GetFilteredJoints() {return &m_FilteredJoints[0];}

    private:
        XMVECTOR m_FilteredJoints[NUI_SKELETON_POSITION_COUNT];
        FilterDoubleExponentialData m_History[NUI_SKELETON_POSITION_COUNT];
        FLOAT m_fSmoothing;
        FLOAT m_fCorrection;
        FLOAT m_fPrediction;
        FLOAT m_fJitterRadius;
        FLOAT m_fMaxDeviationRadius;       
    };

    class FilterCombinationData
    {
    public:
        static const DWORD MAX_COMBINATION_FILTER_TAPS = 10;
        XMVECTOR	m_vPrevDeltas[ MAX_COMBINATION_FILTER_TAPS ];	
        XMVECTOR	m_vWantedPos;					
        XMVECTOR	m_vLastWantedPos;				
        XMVECTOR	m_vPos;						
        FLOAT	    m_fWantedLocalBlendRate;
        FLOAT	    m_fActualLocalBlendRate;
        BOOL	    m_bActive[3];					// Primary/secondary/tertiary

    };

    class FilterCombination
    {
    public:
        FilterCombination() {Init();}

        VOID Init( FLOAT fDefaultApplyRate = 0.055f, 
                   FLOAT fDotProdThreshold = 0.20f,
                   FLOAT fDeltaLengthThreshold = .002f,
                   FLOAT fBlendedLengthThreshold = .005f,
                   BOOL bDotProdNormalize = TRUE,
                   INT nUseTaps = 4,
                   FLOAT fDownBlendRate = .5f,
                   FLOAT fBlendBlendRate = .1f
                )
        {
            m_fDefaultApplyRate = fDefaultApplyRate;
            m_fDotProdThreshold = fDotProdThreshold;
            m_fDeltaLengthThreshold = fDeltaLengthThreshold;
            m_fBlendedLengthThreshold = fBlendedLengthThreshold;
            m_bDotProdNormalize = bDotProdNormalize;
            m_nUseTaps = nUseTaps;
            m_fDownBlendRate = fDownBlendRate;
            m_fBlendBlendRate = fBlendBlendRate;
            Reset();
        }

        VOID Reset()
        {
            XMemSet(m_FilteredJoints, 0, sizeof(m_FilteredJoints));
            XMemSet(m_History, 0, sizeof(m_History));
        }

        VOID Update( const XMVECTOR* pJointPositions );

        XMVECTOR* GetFilteredJoints() {return &m_FilteredJoints[0];}

    private:
        XMVECTOR m_FilteredJoints[NUI_SKELETON_POSITION_COUNT];
        FilterCombinationData m_History[NUI_SKELETON_POSITION_COUNT];
          
        FLOAT m_fDefaultApplyRate;
        FLOAT m_fDotProdThreshold;
        FLOAT m_fDeltaLengthThreshold;
        FLOAT m_fBlendedLengthThreshold;
        BOOL  m_bDotProdNormalize;
        UINT  m_nUseTaps;
        FLOAT m_fDownBlendRate;
        FLOAT m_fBlendBlendRate;

    };
}

#endif