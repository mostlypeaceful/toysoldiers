#include "BasePch.hpp"
#include "tIK.hpp"
#include "tSceneGraph.hpp"
#include "tSpatialEntity.hpp"
#include "Logic/tAnimatable.hpp"

#include "cml/cml.h"

using namespace Sig::Math;

namespace Sig { namespace IK {

	devvar_clamp( u32, Debug_IK_ConstraintOrder, 3, 0, 11, 0 );
	devvar_clamp( u32, Debug_IK_TotalSteps, 10, 1, 30, 0 );
	devvar_clamp( u32, Debug_IK_RotationSteps, 5, 1, 30, 0 );
	devvar( bool, Debug_IK_TryOrientation, true );
	devvar( bool, Debug_IK_TryPosition, true );
	devvar( bool, Debug_IK_DoHinge, true );
	devvar( bool, Debug_IK_FreeAfterLostContact, false );
	devvar( f32, Debug_IK_ErrorTolerance, 0.00f );
	devvar( f32, Debug_IK_BlendOrientation, 0.5f );
	devvar( f32, Debug_IK_BlendPosition, 0.5f );

	devvar( bool, Debug_IK_Render, false );
	devvar( bool, Debug_IK_Override, false );

	// These values should only be used to real time determine what the values should be set to in script.
	// The values should then be migrated to the script and these zerod out
	devvar_clamp( f32, Debug_IK_RayLengthTweak, 0.0f, -cInfinity, cInfinity, 2 );
	devvar_clamp( f32, Debug_IK_ExtraRayTweak, 0.0f, -cInfinity, cInfinity, 2 );
	devvar_clamp( f32, Debug_IK_FootHeightTweak, 0.0f, -cInfinity, cInfinity, 2 );



	// This is basically used for the effector, to reorient the transform so that it is object aligned in bind pose, and back again.
	void tIKJoint::fToObjectSpace( tMat3f& mat ) const
	{
		tVec3f pos = mat.fGetTranslation( );
		mat = mat * tMat3f( mBindPoseToLocal );
		mat.fSetTranslation( pos );
	}

	void tIKJoint::fToOriginalSpace( tMat3f& mat ) const
	{
		tVec3f pos = mat.fGetTranslation( );
		mat = mat * tMat3f( mLocalToBindPose );
		mat.fSetTranslation( pos );
	}

	void tIKJoint::fDetermineFlags( )
	{
		b32 activeAxes[ 3 ];
		u32 activeAxesCnt = 0;
		u32 potentialHingeAxis = ~0;
		b32 allJointsInfinite = true;

		for( u32 i = 0; i < 3; ++i )
		{
			activeAxes[ i ] = !fEqual( mAxesLimitsLow[ i ], mAxesLimitsHigh[ i ] );
			if( mAxesLimitsLow[ i ] != -cInfinity || mAxesLimitsHigh[ i ] != cInfinity ) allJointsInfinite = false;

			if( activeAxes[ i ] ) 
			{
				potentialHingeAxis = i;
				++activeAxesCnt;
			}
		}

		switch( activeAxesCnt )
		{
		case 0:
			mFlags = fSetBits( mFlags, tIKJoint::cFixedJointFlag );
			break;
		case 1:
		case 2:
			mFlags = fSetBits( mFlags, tIKJoint::cHingeJointFlag );
			mHingeAxis = 1; //potentialHingeAxis;
			break;
		default:
			if( allJointsInfinite ) mFlags = fSetBits( mFlags, tIKJoint::cFreeJointFlag );
			else mFlags = fSetBits( mFlags, tIKJoint::cConeJointFlag );
			break;
		}
	}

	void tIKJoint::fSetLength( f32 len )
	{
		mLength = len;
		mLenSqr = mLength * mLength;
	}

	void tIKJoint::fIncLength( f32 incLen )
	{
		mLength += incLen;
		mLenSqr = mLength * mLength;
	}

	tAxisAnglef fAABetweenVecs( const tVec3f& a, const tVec3f& b )
	{
		tVec3f axis = a.fCross( b );
		f32 axisLen = axis.fLength( );

		if( fEqual( axisLen, 0.f ) ) 
			return tAxisAnglef( tVec3f::cYAxis, 0.f );

		axis /= axisLen;
		f32 angle = fAcos( a.fDot( b ) );

		return tAxisAnglef( axis, angle );
	}

	cml::matrix33f_c fToCML( const tMat3f& mat )
	{
		tVec3f aX = mat.fXAxis( );
		tVec3f aY = mat.fYAxis( );
		tVec3f aZ = mat.fZAxis( );
		return cml::matrix33f_c( aX.x, aX.y, aX.z, aY.x, aY.y, aY.z, aZ.x, aZ.y, aZ.z );
	}

	tMat3f fToMat3( const cml::matrix33f_c& mat )
	{
		return tMat3f( tVec3f( mat( 0, 0 ), mat( 0, 1 ), mat( 0, 2 ) )
			, tVec3f( mat( 1, 0 ), mat( 1, 1 ), mat( 1, 2 ) )
			, tVec3f( mat( 2, 0 ), mat( 2, 1 ), mat( 2, 2 ) )
			, tVec3f::cZeroVector );
	}

	tEulerAnglesf fDecomposeLocal( tIKJoint& joint )
	{
		u32 order = joint.mAxesLimitsOrder; //joint.mID == 2 ? Debug_IK_ConstraintOrder : joint.mAxesLimitsOrder;

		tMat3f rot( joint.mLocalDelta );
		cml::matrix33f_c m = fToCML( rot );

		tEulerAnglesf ea;
		cml::matrix_to_euler( m, ea.x, ea.y, ea.z, (cml::EulerOrder)order );

		return ea;
	}

	void fComposeLocal( tIKJoint& joint, const tEulerAnglesf& ea )
	{
		u32 order = joint.mAxesLimitsOrder; //joint.mID == 2 ? Debug_IK_ConstraintOrder : joint.mAxesLimitsOrder;

		cml::matrix33f_c m;
		cml::matrix_rotation_euler( m, ea.x, ea.y, ea.z, (cml::EulerOrder)order );

		tMat3f rot = fToMat3( m );
		joint.mLocalDelta = tQuatf( rot );
	}






	tIKChain::tIKChain( )
		: mLimbLength( 0.f )
		, mWorldRoot( tMat3f::cIdentity )
	{
	}

	void tIKChain::fClampLocal( tIKJoint& joint )
	{
		if( Debug_IK_DoHinge && fTestBits( joint.mFlags, tIKJoint::cHingeJointFlag ) )
		{
			//todo make this work with any axis depending on which axis is the hinge axis.
			log_assert( joint.mHingeAxis == 1, "IK: Hinge currently only works for Y axis! :(" );

			tMat3f localDelta( joint.mLocalDelta );
			tVec3f localRef = localDelta.fXAxis( );
			localRef.y = 0;
			localRef.fNormalizeSafe( tVec3f::cXAxis );
			
			f32 angle = fAtan2( localRef.z, localRef.x );
			angle = fClamp( angle, joint.mAxesLimitsLow[ joint.mHingeAxis ], joint.mAxesLimitsHigh[ joint.mHingeAxis ] );

			joint.mLocalDelta = tQuatf( tAxisAnglef( tVec3f::cYAxis, -angle ) );
		}
		else if( fTestBits( joint.mFlags, tIKJoint::cConeJointFlag ) )
		{
			tMat3f trans = joint.mWorldTransform;
			tVec3f pos = trans.fGetTranslation( );
			trans = trans * tMat3f( joint.mBindPoseToLocal );
			trans.fSetTranslation( pos );

			/*tMat3f localDelta( joint.mLocalDelta * joint.mBindPoseToLocal );
			tVec3f localRef = localDelta.fZAxis( );

			localRef.x = 0;
			localRef.fNormalizeSafe( tVec3f::cZAxis );

			localDelta.fOrientZAxis(localRef, tVec3f::cYAxis );
			joint.mLocalDelta = tQuatf( localDelta ) * joint.mLocalToBindPose;*/

			pos = trans.fGetTranslation( );
			trans = trans * tMat3f( joint.mLocalToBindPose );
			trans.fSetTranslation( pos );

			fSetWorldOrientation( trans, joint );
		}
		else if( !fTestBits( joint.mFlags, tIKJoint::cFreeJointFlag ) )
		{
			tEulerAnglesf angles = fDecomposeLocal( joint );

			for( u32 a = 0; a < 3; ++ a)
				angles[ a ] = fClamp( angles[ a ], joint.mAxesLimitsLow[ a ], joint.mAxesLimitsHigh[ a ] );

			fComposeLocal( joint, angles );
		}
	}

	tMat3f tIKChain::fGetBoneParentWorldSpace( const tIKJoint& joint ) const
	{
		tMat3f parent = joint.mID == 0 ? mWorldRoot : mJoints[ joint.mID - 1 ].mWorldTransform;
		parent *= joint.mBindParentRelative;

		return parent;
	}

	void tIKChain::fSetWorldOrientation( const tMat3f& worldR, tIKJoint& joint )
	{
		// Compute new local delta
		tMat3f parent = fGetBoneParentWorldSpace( joint );
		tMat3f parentInv = parent.fInverse( );

		joint.mLocalDelta = tQuatf( parentInv * worldR );
		joint.mLocalDelta.fNormalizeSafe( tQuatf::cIdentity );
	}

	tSolveResult tIKChain::fSolve( const Math::tMat3f& root, const Math::tMat3f& effectorTarget, f32 dt, tLogic& logic )
	{
		if( mJoints.fCount( ) == 0 ) return cSolveResultFailed;

		mWorldRoot = root;
		fForwardSolveToEffector( 0 );

		const tVec3f targetP = effectorTarget.fGetTranslation( );
		
		tVec3f errorVec = targetP - mEndEffector.fGetTranslation( );
		f32 errorMagSqr = errorVec.fLengthSquared( );
		f32 errorToleranceSqr = Debug_IK_ErrorTolerance * Debug_IK_ErrorTolerance;

		for( u32 step = 0; step < Debug_IK_TotalSteps; ++step )
		{
			if( errorMagSqr <= errorToleranceSqr )
				break;

			for( u32 i = 0; i < mPriority.fCount( ); ++i )
			{
				const u32 jointIndex = mPriority[ i ];
				tIKJoint &joint = mJoints[ jointIndex ];

				if( fTestBits( joint.mFlags, tIKJoint::cFixedJointFlag ) )
				{
					joint.mLocalDelta = tQuatf::cIdentity;
					continue;
				}

				tMat3f newWorldOrienation = joint.mWorldTransform;

				if( step < Debug_IK_RotationSteps && Debug_IK_TryOrientation )
				{
					// Align the end effector orientation with the target
					tMat3f effectorDelta = effectorTarget * mEndEffector.fInverse( );
					tAxisAnglef aa = tAxisAnglef( tQuatf( effectorDelta ) );
					aa.mAngle *= Debug_IK_BlendOrientation;
					effectorDelta = tMat3f( tQuatf( aa ) );

					newWorldOrienation = effectorDelta * joint.mWorldTransform;

					fSetWorldOrientation( newWorldOrienation, joint );
					fClampLocal( joint );
					fForwardSolveToEffector( jointIndex );
				}

				if( Debug_IK_TryPosition && jointIndex != mJoints.fCount( )-1 )
				{
					// Align these two vectors to correct effector position error
					const tVec3f jointP = joint.mWorldTransform.fGetTranslation( );
					const tVec3f effectorP = mEndEffector.fGetTranslation( );
					tVec3f jointToTarget = targetP - jointP; 
					tVec3f jointToEnd = effectorP - jointP;
					f32 jTTLen = jointToTarget.fLength( );
					f32 jTELen = jointToEnd.fLength( );

					if( jTTLen > 0.001f && jTELen > 0.001f )
					{
						// Rotate locally to align effector position
						tAxisAnglef change = fAABetweenVecs( jointToEnd.fNormalize( ), jointToTarget.fNormalize( ) );
						change.mAngle *= Debug_IK_BlendPosition;
						newWorldOrienation = tMat3f( tQuatf( change ) ) * newWorldOrienation;

						if( Debug_IK_Render && jointIndex == 2 )
						{
							logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint( jointP ), logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint(jointP + change.mAxis * change.mAngle), tVec4f(1,1,0,0.5f) );
							logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint( jointP ), logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint(jointP + jointToTarget), tVec4f(1,1,0,0.5f) );
							//logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint( jointP ), logic.fOwnerEntity( )->fObjectToWorld( ).fXformPoint(jointP + jointToEnd), tVec4f(1,0,0,0.5f) );
						}

						fSetWorldOrientation( newWorldOrienation, joint );
						fClampLocal( joint );
						fForwardSolveToEffector( jointIndex );
					}
				}

				errorVec = targetP - mEndEffector.fGetTranslation( );
				errorMagSqr = errorVec.fLengthSquared( );
			}
		}

		return cSolveResultSolved;
	}

	void tIKChain::fForwardSolveToEffector( u32 bone )
	{
		for( u32 i = 0; i < mJoints.fCount( ); ++i )
			fForwardSolveBone( i );
	}

	void tIKChain::fForwardSolveBone( u32 bone )
	{
		if( bone != 0 ) 
		{
			tMat3f jointDelta = tMat3f( mJoints[ bone ].mLocalDelta );
			mJoints[ bone ].mWorldTransform = mJoints[ bone-1 ].mWorldTransform *  mJoints[ bone ].mBindParentRelative * jointDelta;
		}
		else
		{
			tMat3f jointDelta = tMat3f( mJoints[ bone ].mLocalDelta );
			mJoints[ bone ].mWorldTransform = mWorldRoot * mJoints[ bone ].mBindParentRelative * jointDelta;
		}

		if( bone == mJoints.fCount( )-1 )
			mEndEffector = mJoints[ bone ].mWorldTransform;
	}

	void tIKChain::fRender( const Math::tMat3f& xForm, const Math::tMat3f& root, const Math::tMat3f& endEffector, tLogic& logic )
	{
		if( !Debug_IK_Render ) return;

		//logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( root, 0.5f, 0.5f );
		logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * endEffector, 0.5f, 0.5f );
		logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( (xForm * root).fGetTranslation( ), (xForm * endEffector).fGetTranslation( ), tVec4f(1,1,0,0.5f) );

		for( u32 i = 0; i < mJoints.fCount( ); ++i )
		{
			logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * mJoints[i].mWorldTransform, 0.25f, 1.0f );

			if( i < mJoints.fCount( ) - 1 )
			{
				//tVec4f lineColor(0,0,0,0.5f);
				//lineColor.fAxis( i % 3 ) = 1;

				tVec4f lineColor(1,1,1,0.5f);
				logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( (xForm * mJoints[i].mWorldTransform).fGetTranslation( ), (xForm * mJoints[i+1].mWorldTransform).fGetTranslation( ), lineColor );
			}
		}
	}

	void tIKChain::fAddJoint( const tIKJoint& joint )
	{
		mPriority.fPushBack( mJoints.fCount( ) );
		//mPriority.fPushFront( mJoints.fCount( ) );

		mLimbLength += joint.mLength;
		mJoints.fPushBack( joint );
		mJoints.fBack( ).fDetermineFlags( );
	}

	void tIKChain::fSetupTestData( )
	{
		/*const u32 bCnt = 3;
		tVec3f pos[bCnt] = { tVec3f(0), tVec3f( 0, -1.f, 0.5f ), tVec3f( 0, -1.5f, 0.5f ), tVec3f( 0, -2.f, 0 ) };*/
		const u32 bCnt = 4;
		tVec3f pos[bCnt] = { tVec3f(0.25f,0,0), tVec3f( 0.25f, -0.66f, 0.5f ), tVec3f( 0.25f, -1.2f, 0.5f ), tVec3f( 0.25f, -2.f, 0 ) };

		for( u32 i = 0; i < bCnt; ++i )
		{
			tIKJoint joint;
			//joint.mAxesLimitsLow = tEulerAnglesf( -c2Pi );
			//joint.mAxesLimitsHigh = tEulerAnglesf( c2Pi );
			joint.mAxesLimitsLow = tVec3f( 0 );
			joint.mAxesLimitsHigh = tVec3f( 0 );
			joint.mAxesLimitsOrder = cml::euler_order_xyz;
			joint.mID = i;

			// setup bones like jason does
			if( i != bCnt -1 )
			{
				// X along bone axis
				tVec3f x = pos[	i+1 ] - pos[ i ];
				x.fNormalize( );
				tVec3f y = tVec3f::cXAxis;
				tVec3f z = x.fCross( y );
				z.fNormalize( );

				joint.mWorldTransform = tMat3f( x, y, z, pos[i] );

				if( i == 0 )
				{
					joint.mAxesLimitsOrder = cml::euler_order_xzy;
					joint.mAxesLimitsLow.x = -c2Pi; joint.mAxesLimitsHigh.x  = c2Pi;
					joint.mAxesLimitsLow.z = -c2Pi; joint.mAxesLimitsHigh.z  = c2Pi;
				}

				if( i == 0 || i == 1 || i == 2 )
				{
					joint.mAxesLimitsOrder = cml::euler_order_xyz;
					joint.mAxesLimitsLow.y = -c2Pi; joint.mAxesLimitsHigh.y = c2Pi;
				}
			}
			else
			{
				// ankle aligned to world
				joint.mWorldTransform = tMat3f::cIdentity;
				joint.mWorldTransform.fSetTranslation( pos[ i ] );
				joint.mAxesLimitsOrder = cml::euler_order_xyz;
				joint.mAxesLimitsLow.x = -c2Pi; joint.mAxesLimitsHigh.x = c2Pi;
				//joint.mAxesLimitsLow.y = -c2Pi; joint.mAxesLimitsHigh.y = c2Pi;
				joint.mAxesLimitsLow.z = -c2Pi; joint.mAxesLimitsHigh.z = c2Pi;
			}

			tMat3f parentInv = i == 0 ? tMat3f::cIdentity : mJoints[ i - 1].mWorldTransform.fInverse( );
			joint.mBindParentRelative = parentInv * joint.mWorldTransform;

			fAddJoint( joint );
		}
	}


	f32 fLengthBetweenTransforms( const tMat3f& a, const tMat3f& b )
	{
		return (a.fGetTranslation( ) - b.fGetTranslation( )).fLength( );
	}

	tIKLimb::tIKLimb( )
		: mLimbLength( 0.f )
		, mWorldRoot( tMat3f::cIdentity )
	{
		for( u32 i = 0; i < cJointCount; ++i )
			mJointIndices[ i ] = ~0;
	}

	void tIKLimb::fComputeCombinedJointInfo( tJoints from, tJoints to )
	{
		tVec3f fromT = fJoint( from ).mWorldTransform.fGetTranslation( );
		tVec3f toT = fJoint( to ).mWorldTransform.fGetTranslation( );
		tVec3f totalUpperDirection = toT - fromT;

		f32 totalUpperLen;
		totalUpperDirection.fNormalize( totalUpperLen );

		fJoint( from ).fSetLength( totalUpperLen );
		mLimbLength += totalUpperLen;

		tMat3f limbSpace = tMat3f::cIdentity;
		limbSpace.fOrientZWithXAxis( totalUpperDirection, tVec3f::cXAxis );

		tMat3f limbToBind = limbSpace.fInverse( ) * fJoint( from ).mWorldTransform;
		fJoint( from ).mLimbToBindSpace = tQuatf( limbToBind );
	}

	void tIKLimb::fAddJoint( const tIKJoint& joint )
	{
		u32 jointIndex = mJoints.fCount( );
		mJoints.fPushBack( joint );

		tIKJoint& j = mJoints.fBack( );
		j.fDetermineFlags( );

		f32 newLength = j.mBindParentRelative.fGetTranslation( ).fLength( );
		if( jointIndex > 0 )
			mJoints[ jointIndex - 1 ].fSetLength( newLength );

		if( mJointIndices[ cUpperJoint ] == ~0 )
		{
			mJointIndices[ cUpperJoint ] = jointIndex; //obviously always zero
		}
		else if( mJointIndices[ cLowerJoint ] == ~0 )
		{
			if( fTestBits( j.mFlags, tIKJoint::cHingeJointFlag ) )
			{
				mJointIndices[ cLowerJoint ] = jointIndex;
				fComputeCombinedJointInfo( cUpperJoint, cLowerJoint );
			}
		}
		else if( mJointIndices[ cEndJoint ] == ~0 )
		{
			//if( fTestBits( j.mFlags, tIKJoint::cConeJointFlag ) )
			// for now assume next joint is end joint :/
				mJointIndices[ cEndJoint ] = jointIndex;

			fComputeCombinedJointInfo( cLowerJoint, cEndJoint );
			fJoint( cEndJoint ).mLimbToBindSpace = fJoint( cEndJoint ).mLocalToBindPose;
		}
		else
			log_assert( false, "I have no business knowing about this joint." );
	}

	tSolveResult tIKLimb::fSolve( const Math::tMat3f& root, const Math::tMat3f& endEffector, f32 dt, tLogic& logic )
	{
		mWorldRoot = root;

		tIKJoint &upperJoint = fJoint( cUpperJoint );
		tIKJoint &lowerJoint = fJoint( cLowerJoint );
		tIKJoint &endJoint = fJoint( cEndJoint );

		tMat3f effectorLocal = endEffector * tMat3f( endJoint.mBindPoseToLocal );
		tMat3f upperBind = root * upperJoint.mBindParentRelative;
		tVec3f rootP = upperBind.fGetTranslation( );
		tVec3f endP = endEffector.fGetTranslation( );
		tVec3f direction = endP - rootP;
		f32 hypotenuse;
		direction.fNormalizeSafe( tVec3f::cZeroVector, hypotenuse );

		tSolveResult solved = cSolveResultFailed;
		f32 upperAng = 0;
		f32 lowerAng = 0;

		if( hypotenuse > 0.f &&  hypotenuse <= fLimbLength( ) )
		{
			f32 hypotSqr = hypotenuse * hypotenuse;

			// apply law of cosines to find lower joint angle
			// a^2 = b^2 + c^2 - 2bc * cos(A)
			// (a^2 - b^2 - c^2) / -2bc = cos( A )
			// an angle of cPI is equal to no bone rotation, 0 angle is cPI bone rotation
			lowerAng = Math::cPi - acos( (hypotSqr - upperJoint.mLenSqr - lowerJoint.mLenSqr) / (-2 * upperJoint.mLength * lowerJoint.mLength) );

			// apply law of sines to find upper joint angle relative to direction vector
			// a / sin(A) = b / sin(B)
			// sin(A) = a * sin(B) / b;
			f32 val = fSin( lowerAng );
			f32 val2 = lowerJoint.mLength * val / hypotenuse;
			upperAng = fAsin( val2 );

			if( lowerJoint.mLenSqr - upperJoint.mLenSqr > hypotSqr ) 
				upperAng = cPi - upperAng;

			solved = cSolveResultSolved;
		}
		else
			solved = cSolvedResultSolvedNotIdeal;

		tMat3f upperTransform( tQuatf( tAxisAnglef( tVec3f::cXAxis, -upperAng ) ), tVec3f(0,0,0) );
		tMat3f lowerTransform( tQuatf( tAxisAnglef( tVec3f::cXAxis, lowerAng ) ), tVec3f(0,0, upperJoint.mLength) );
		tMat3f endTransform( tQuatf::cIdentity, tVec3f(0,0,lowerJoint.mLength) );

		// new root transform
		tVec3f z = direction;
		tVec3f y = z.fCross( effectorLocal.fXAxis( ) ).fNormalizeSafe( tVec3f::cXAxis );
		tVec3f x = y.fCross( z );
		tMat3f newRoot( x, y, z, rootP );

		// new joint transforms
		tMat3f upperNew = newRoot * upperTransform;									//new transform in limb space
		tMat3f upperNewBind = upperNew * tMat3f( upperJoint.mLimbToBindSpace );		//new transform in bound space
		tMat3f upperDelta = upperBind.fInverse( ) * upperNewBind;
		upperJoint.mLocalDelta = tQuatf( upperDelta );

		tMat3f lowerNew = upperNew * lowerTransform;
		tMat3f lowerNewBind = lowerNew * tMat3f( lowerJoint.mLimbToBindSpace );
		tMat3f lowerBind = upperNewBind * lowerJoint.mBindParentRelative;
		tMat3f lowerDelta = lowerBind.fInverse( ) * lowerNewBind;
		lowerJoint.mLocalDelta = tQuatf( lowerDelta );

		tMat3f endNewBind = endEffector; //already in bind space
		tMat3f endBind = lowerNewBind * endJoint.mBindParentRelative;
		tMat3f endDelta = endBind.fInverse( ) * endNewBind;
		endJoint.mLocalDelta = tQuatf( endDelta );

		// The old way, for reference.
		//{
		//	upperJoint.mWorldTransform = newRoot * upperTransform;
		//	lowerJoint.mWorldTransform = upperJoint.mWorldTransform * lowerTransform;
		//	endJoint.mWorldTransform = lowerJoint.mWorldTransform * endTransform;
		//	//endJoint.mWorldTransform = endEffector;
		//}

		fForwardSolveToEffector( 0 );

		return solved;
	}

	void tIKLimb::fForwardSolveToEffector( u32 bone )
	{
		for( u32 i = 0; i < mJoints.fCount( ); ++i )
			fForwardSolveBone( i );
	}

	void tIKLimb::fForwardSolveBone( u32 bone )
	{
		if( bone != 0 ) 
		{
			tMat3f jointDelta = tMat3f( mJoints[ bone ].mLocalDelta );
			mJoints[ bone ].mWorldTransform = mJoints[ bone-1 ].mWorldTransform *  mJoints[ bone ].mBindParentRelative * jointDelta;
		}
		else
		{
			tMat3f jointDelta = tMat3f( mJoints[ bone ].mLocalDelta );
			mJoints[ bone ].mWorldTransform = mWorldRoot * mJoints[ bone ].mBindParentRelative * jointDelta;
		}

		if( bone == mJoints.fCount( )-1 )
			mEndEffector = mJoints[ bone ].mWorldTransform;
	}

	void tIKLimb::fRender( const Math::tMat3f& xForm, const Math::tMat3f& root, const Math::tMat3f& endEffector, tLogic& logic )
	{
		if( !Debug_IK_Render ) return;

		logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * root, 0.5f, 0.5f );
		//logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( endEffector, 0.5f, 0.5f );
		logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( (xForm * root).fGetTranslation( ), (xForm * endEffector).fGetTranslation( ), tVec4f(1,1,0,0.5f) );

		u32 jCnt = mJoints.fCount( );
		for( u32 i = 0; i < jCnt; ++i )
		{
			logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * mJoints[i].mWorldTransform, 0.25f, 1.0f );

			if( i < jCnt - 1 )
			{
				tVec4f lineColor(0,0,0,1);
				lineColor.fAxis( i % 3 ) = 1;
				logic.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( (xForm * mJoints[i].mWorldTransform).fGetTranslation( ), (xForm * mJoints[i+1].mWorldTransform).fGetTranslation( ), lineColor );
			}
		}
	}

	void tIKLimb::fSetupTestData( )
	{
	}
	
	b32 fOverride( )
	{
		return Debug_IK_Override;
	}

	namespace 
	{
		struct tIKRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tEntity*					mIgnoreEntity;
			tEntityTagMask				mIgnoreTags;
			tEntityTagMask				mRequireTags;

			explicit tIKRayCastCallback( tEntity& ignore, tEntityTagMask ignoreTags, tEntityTagMask requireTags  ) 
				: mFirstEntity( 0 )
				, mIgnoreEntity( &ignore )
				, mIgnoreTags( ignoreTags )
				, mRequireTags( requireTags ) 
			{ }
			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
				if( !spatial->fHasGameTagsAny( mRequireTags ) )
					return;
				if( spatial->fHasGameTagsAny( mIgnoreTags ) )
					return;
				if( spatial == mIgnoreEntity )
					return;
				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};
	}


	tCharacterLegTargets::tCharacterLegTargets( tLogic *owner, tEntityTagMask ignoreTags, tEntityTagMask requireTags, b32 doLock )
		: mOwner( owner )
		, mRayLength( 0.f )
		, mFootHeight( 0.f )
		, mExtraRay( 0.f )
		, mIgnoreTags( ignoreTags )
		, mRequireTags( requireTags )
		, mIdle( false )
		, mIdleLastFrame( false )
		, mDoLock( doLock )
		, mResetLocks( false )
		, mResetLocksTimer( 0.f )
	{ 
		for( u32 i = 0; i < cTargetChannelCount; ++i )
		{
			mLockMode[ i ] = cLockModeDetect;
			mLockedStatus[ i ] = cLockNone;
			mBlendSrc[ i ].fSetBlends( 0.3f );
			mLockDownPos[ i ] = tPRSXformf::cZeroXform;
			mLockDownBlend[ i ] = tPRSXformf::cIdentity;
		}
	}

	void tCharacterLegTargets::fCallbackMT( tIKCallbackArgs& args )
	{
		const f32 dt = args.mDT;
		log_assert( mRayLength != 0.f, "You forgot to initialize variables of tCharacterLegTargets" );

		const f32 rayLen = mRayLength + Debug_IK_RayLengthTweak;
		const f32 footHeight = mFootHeight + Debug_IK_FootHeightTweak;
		const f32 extraRay = mExtraRay + Debug_IK_ExtraRayTweak;

		const tVec3f extent( 0, -rayLen, 0 );
		const tVec3f originOffset = -extent - tVec3f( 0, footHeight + extraRay, 0 );
		tVec3f footPos = args.mObjectSpaceEffectorTarget.fGetTranslation( );

		const tMat3f& xForm = mOwner->fOwnerEntity( )->fObjectToWorld( );
		const tMat3f& xFormInv = mOwner->fOwnerEntity( )->fWorldToObject( );
		tRayf rayFromFoot;
		rayFromFoot.mExtent = extent;
		rayFromFoot.mOrigin = footPos + originOffset;
		rayFromFoot = rayFromFoot.fTransform( xForm );

		f32 blendTarget = 0.f;
		u16& lockMode = mLockMode[ args.mChannel ];
		u16& lockStatus = mLockedStatus[ args.mChannel ];

		if( lockStatus != cLockNone && args.mSolveResult != cSolveResultSolved )
		{
			lockStatus = cLockNone;

			if( lockMode != cLockModeLock )
				lockMode = Debug_IK_FreeAfterLostContact ? cLockModeFree : cLockModeDetect;
		}

		tIKRayCastCallback callback( *mOwner->fOwnerEntity( ), mIgnoreTags, mRequireTags );
		mOwner->fSceneGraph( )->fRayCast( rayFromFoot, callback );

		if( callback.mHit.fHit( ) )
		{
			if( mDoLock && lockStatus != cLockNone )
			{
				args.mChanged = true;
			}
			else if( lockMode != cLockModeFree )
			{
				if( args.mSolveResult == cSolveResultSolved && lockMode == cLockModeLock ) 
					lockStatus = cLockPrimary;

				tVec3f pt = rayFromFoot.fEvaluate( callback.mHit.mT ) + callback.mHit.mN * footHeight;

				// X axis ref
				tVec3f refAxis = xForm.fXformVector( args.mObjectSpaceEffectorTarget.fXAxis( ) );
				tVec3f y = callback.mHit.mN;
				tVec3f z = refAxis.fCross( y );
				z.fNormalizeSafe( tVec3f::cZAxis );
				tVec3f x = y.fCross( z );

				tMat3f target = tMat3f( x, y, z, pt );
				fSetTarget( args.mChannel, args.mIKBlendStrength, target );

				args.mChanged = true;
			}
		}
		else if ( mIdle && lockMode != cLockModeFree )
		{
			//no hit, shoot from hip, only on idle
			blendTarget = 1.0f;

			tVec3f hipPos = args.mObjectSpaceBoneFirstBone.fGetTranslation( );
			hipPos.y = footPos.y;

			footPos = fLerp( footPos, hipPos, mBlendSrc[ args.mChannel ].fValue( ) );

			tRayf rayFromHip;
			rayFromHip.mExtent = extent;
			rayFromHip.mOrigin = footPos + originOffset;
			rayFromHip = rayFromHip.fTransform( xForm );

			tIKRayCastCallback callback( *mOwner->fOwnerEntity( ), mIgnoreTags, mRequireTags );
			mOwner->fSceneGraph( )->fRayCast( rayFromHip, callback );

			b32 lockedS = mDoLock && (lockStatus == cLockSecondary);

			if( callback.mHit.fHit( ) || lockedS )
			{
				log_assert( !callback.mHit.mN.fIsNan( ), "Someones not returning a normal!" );

				if( !lockedS )
				{
					lockStatus = cLockSecondary;
					tVec3f pt = rayFromHip.fEvaluate( callback.mHit.mT ) + callback.mHit.mN * footHeight;

					// X axis ref (same as above, should be function :( )
					tVec3f refAxis = xForm.fXformVector( args.mObjectSpaceEffectorTarget.fXAxis( ) );
					tVec3f y = callback.mHit.mN;
					tVec3f z = refAxis.fCross( y );
					z.fNormalizeSafe( tVec3f::cZAxis );
					tVec3f x = y.fCross( z );

					tMat3f target = tMat3f( x, y, z, pt );
					fSetTarget( args.mChannel, args.mIKBlendStrength, target );
				}

				args.mChanged = true;
			}
			else
				lockStatus = cLockNone;

			if( Debug_IK_Render ) 
			{
				tVec4f color( 0, 0, 0, 1 );
				if( callback.mHit.fHit( ) ) color.fAxis( 1 ) = 1;
				else color.fAxis( 0 ) = 1;

				mOwner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( rayFromHip, color );
				mOwner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * args.mObjectSpaceBoneFirstBone, 1.0f, 1.0f  );
			}
		}
		else
		{
			if( lockMode != cLockModeLock )
			{
				lockStatus = cLockNone;
				lockMode = Debug_IK_FreeAfterLostContact ? cLockModeFree : cLockModeDetect;
			}
		}

		//mBlend[ args.mChannel ].fSetBlends( Gameplay_Character_IK_FootTargetBlend );
		mBlendSrc[ args.mChannel ].fStep( blendTarget, dt );

		f32 dstBlend = args.mChanged ? 1.f : 0.f;
		mBlendDst[ args.mChannel ].fSetBlends( 0.5f );
		mBlendDst[ args.mChannel ].fStep( dstBlend, dt );

		mLockDownBlend[ args.mChannel ].fBlendNLerp( mLockDownPos[ args.mChannel ], 0.5f );

		tPRSXformf output( xForm * args.mObjectSpaceEffectorTarget );
		output.fBlendNLerp( mLockDownBlend[ args.mChannel ], mBlendDst[ args.mChannel ].fValue( ) );

		//if( !args.mChanged )
		//{
		//	//blend the target back to the animation
		//	fSetTarget( args.mChannel, args.mIKBlendStrength, xForm * args.mObjectSpaceEffectorTarget );
		//	args.mChanged = true;
		//}

		//if( args.mChanged )
			args.mObjectSpaceEffectorTarget = xFormInv * tMat3f( output );
			args.mChanged = true;
		
		if( Debug_IK_Render ) 
		{
			tVec4f color( 0, 0, 0, 1 );
			if( callback.mHit.fHit( ) ) color.fAxis( 1 ) = 1;
			else color.fAxis( 0 ) = 1;

			mOwner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( rayFromFoot, color );
			mOwner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( xForm * args.mObjectSpaceEffectorTarget, 1.0f, 1.0f  );
		}

		mIdleLastFrame = mIdle;

		// Reset locks will relax the feet once the user has idled for a while. (only once)
		if( mResetLocks )
		{
			mResetLocksTimer += dt;
			if( mResetLocksTimer > 1.0f )
			{
				mResetLocks = false;
				if( mIdle ) 
				{
					mLockedStatus[ cTargetChannelRightLeg ] = cLockNone;
					mLockedStatus[ cTargetChannelLeftLeg ] = cLockNone;
				}
			}
		}
	}

	void tCharacterLegTargets::fSetTarget( u32 channel, f32 ikBlend, const Math::tMat3f& target )
	{
		mLockDownPos[ channel ] = tPRSXformf( target );

		const f32 cCantSeeEpsilon = 0.1f;
		if( mBlendDst[ channel ].fValue( ) < cCantSeeEpsilon )
			mLockDownBlend[ channel ] = mLockDownPos[ channel ];
		//else
		//{
		//	log_line( "blending..." );
		//}
	}

	void tCharacterLegTargets::fSetIdle( b32 idle )
	{
		mIdle = idle;

		if( mIdle && !mIdleLastFrame )
		{
			fSetLockOnFirstContactMode( cTargetChannelLeftLeg );
			fSetLockOnFirstContactMode( cTargetChannelRightLeg );
		}
		else if( !mIdle && mIdleLastFrame )
		{
			mResetLocksTimer = 0.f;
			mResetLocks = true;
		}
	}

	namespace 
	{
		const tStringPtr cIKLeftLegLock( "IKLeftLegLock" );
		const tStringPtr cIKLeftLegUnlock( "IKLeftLegUnlock" );
		const tStringPtr cIKLeftLegDetect( "IKLeftLegDetect" );
		const tStringPtr cIKRightLegLock( "IKRightLegLock" );
		const tStringPtr cIKRightLegUnlock( "IKRightLegUnlock" );
		const tStringPtr cIKRightLegDetect( "IKRightLegDetect" );
	}

	b32 tCharacterLegTargets::fHandleLogicEvent( const Logic::tEvent& e )
	{
		const tKeyFrameEventContext* context = e.fContext< tKeyFrameEventContext >( );
		sigassert( context );

		if( context->mTag == cIKLeftLegLock )			fSetLockOnFirstContactMode( cTargetChannelLeftLeg );
		else if( context->mTag == cIKLeftLegUnlock )	fSetFreeAfterLostContactMode( cTargetChannelLeftLeg ); 
		else if( context->mTag == cIKLeftLegDetect )	fSetDetectMode( cTargetChannelLeftLeg );
		else if( context->mTag == cIKRightLegLock )		fSetLockOnFirstContactMode( cTargetChannelRightLeg );
		else if( context->mTag == cIKRightLegUnlock )	fSetFreeAfterLostContactMode( cTargetChannelRightLeg );
		else if( context->mTag == cIKRightLegDetect )	fSetDetectMode( cTargetChannelRightLeg );
		else return false;

		return true;
	}

	void tCharacterLegTargets::fSetDetectMode( u32 channel )
	{
		mLockMode[ channel ] = cLockModeDetect;
	}

	void tCharacterLegTargets::fSetLockOnFirstContactMode( u32 channel )
	{
		mLockMode[ channel ] = cLockModeLock;
	}

	void tCharacterLegTargets::fSetFreeAfterLostContactMode( u32 channel )
	{
		mLockMode[ channel ] = cLockModeDetect;
	}

	void tCharacterLegTargets::fDebugData( std::stringstream& ss, u32 indentDepth, u32 channel ) const
	{
		const u16& mode = mLockMode[ channel ];
		const u16& status = mLockedStatus[ channel ];

		if( fTestBits( status, cLockSecondary ) )
			ss << "Sec Lock";
		else if( fTestBits( status, cLockPrimary ) )
			ss << "Prim Lock";
		else if( mode == cLockModeLock )
			ss << "Locking";
		else if( mode == cLockModeDetect )
			ss << "Detecting";
		else if( mode == cLockModeFree )
			ss << "Free";
		else 
			ss << "Error";
	}


	void tCharacterLegTargets::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tCharacterLegTargets, tIKCallback, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("RayLength"), &tCharacterLegTargets::mRayLength)
			.Var(_SC("ExtraRay"), &tCharacterLegTargets::mExtraRay)
			.Var(_SC("FootHeight"), &tCharacterLegTargets::mFootHeight)
			;

		vm.fNamespace(_SC("IK")).Bind( _SC("CharacterLegTargets"), classDesc );

		vm.fConstTable( ).Const( _SC("IK_CHANNEL_LEFT_FOOT"), cTargetChannelLeftLeg );
		vm.fConstTable( ).Const( _SC("IK_CHANNEL_RIGHT_FOOT"), cTargetChannelRightLeg );
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tIKCallbackArgs, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
			vm.fNamespace(_SC("IK")).Bind( _SC("IKCallbackArgs"), classDesc );
		}
		{
			Sqrat::Class<tIKCallback, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
			vm.fNamespace(_SC("IK")).Bind( _SC("IKCallback"), classDesc );
		}

		tCharacterLegTargets::fExportScriptInterface( vm );
	}

} }