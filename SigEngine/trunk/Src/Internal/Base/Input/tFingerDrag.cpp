#include "BasePch.hpp"
#include "tFingerDrag.hpp"
#include "StringUtil.hpp"
#include "tTouchImplementationDetails.hpp"
#include "Input/tMouse.hpp"

namespace Sig { namespace Input {
	using namespace ImplDetails;

	tFingerDrag::tFingerDrag()
		: mType(cTypeAbsolute)
		, mFlipX(0)
		, mFlipY(0)
	{
	}

	b32 tFingerDrag::fLoadFromString( const char* s )
	{
		using namespace StringUtil;

		if (!s || !*s)
		{
			// empty cell
			mType = cTypeAbsolute;
			return true;
		}

		s = fReadUntilNonSpace( s );
		b32 invertX=false, invertY=false, anyFinger=false, relative=false, absolute=false;

		while ( s && *s )
		{
			b32 foundFlag
				=	fCheckFlag( s, "Inv", invertY )
				||	fCheckFlag( s, "Any", anyFinger )
				||	fCheckFlag( s, "Rel", relative )
				||	fCheckFlag( s, "Abs", absolute )
				;
			if ( foundFlag )
				continue;

			if ( fIsAnyOf( "0123456789", *s ) )
			{
				mFingerFilter.fSetBit( *s-'0', true );
				++s;
				continue;
			}

			const char* afterWs = fReadUntilNonSpace(s);
			sigassert( afterWs!=s );
			if ( afterWs==s )
				return false;
			s = afterWs;
		}

		int typeFlags = 0;
		if ( relative	) ++typeFlags, mType = cTypeRelative;
		if ( absolute	) ++typeFlags, mType = cTypeAbsolute;
		sigassert( typeFlags == 1 );
		if ( typeFlags != 1 )
			return false;

		mFlipX = invertX;

		if ( anyFinger )
			mFingerFilter.fSetAll( true );

		return true;
	}

	Math::tVec2f tFingerDrag::fEvaluate( const tTouch& touch ) const
	{
		const u32 cHeldFingerCount = touch.fCountFingersHeld( ); //0 1 or 2

		b32 finger1 = mFingerFilter.fGetBit( 1 );
		b32 finger2 = mFingerFilter.fGetBit( 2 );
		tTouch::tFinger desiredFinger = tTouch::cFingerNone;

		if( finger1 && finger2 && ( cHeldFingerCount >= 1 ) )
			desiredFinger = tTouch::cFingersAll;
		else if( finger1 && ( cHeldFingerCount >= 1 ) )
			desiredFinger = tTouch::cFinger1;
		else if( finger2 && ( cHeldFingerCount >= 2) )
			desiredFinger = tTouch::cFinger2;
		else
			return Math::tVec2f( 0, 0 );
		
		Math::tVec2f result( 0, 0);

		switch ( mType )
		{
		case cTypeAbsolute:
			result = touch.fFingerDragThisFrame( desiredFinger );
			break;
		case cTypeRelative:
			result = touch.fFingerDragTotal( desiredFinger );
			break;
		default:
			sigassert(!"Invalid type");
			break;
		}

		if ( mFlipX ) result.x *= -1;
		if ( mFlipY ) result.y *= -1;

		return result;
	}

	Math::tVec2f tFingerDrag::fEvaluate( const tMouse& mouse ) const
	{
		//Which buttons do we require?  We must require at least one.
		b32 leftButtonRequired = mFingerFilter.fGetBit( 1 );
		b32 rightButtonRequired = mFingerFilter.fGetBit( 2 );
		b32 middleButtonRequired = mFingerFilter.fGetBit( 3 );

		//Make sure there is at least one required button.
		if ( !( leftButtonRequired || middleButtonRequired || rightButtonRequired ))
			return Math::tVec2f( 0,0 );

		//Which buttons are held?
		b32 leftButtonHeld = mouse.fButtonHeld( tMouse::cButtonLeft );
		b32 rightButtonHeld = mouse.fButtonHeld( tMouse::cButtonRight );
		b32 middleButtonHeld = mouse.fButtonHeld( tMouse::cButtonMiddle );
		
		//Make sure at least one of the required buttons are held down.
		bool oneRequirementMet = false;

		if ( leftButtonRequired && leftButtonHeld )
			oneRequirementMet = true;
		
		else if ( rightButtonRequired && !rightButtonHeld )
			oneRequirementMet = true;
		
		else if ( middleButtonRequired && !middleButtonHeld)
			oneRequirementMet = true;

		//None of the button requirements were met.  Fail.
		if( !oneRequirementMet )
			return Math::tVec2f( 0,0 );

		Math::tVec2f result = Math::tVec2f::cZeroVector;

		switch ( mType )
		{
		case cTypeAbsolute:
			result = mouse.fDeltaPosition( );
			break;
		case cTypeRelative:
			{
				//Check the button that's down with the following priority: Left, Right, Middle.
				if ( leftButtonHeld )
					result = mouse.fDragDelta( tMouse::cButtonLeft );
				else if ( rightButtonHeld )
					result = mouse.fDragDelta( tMouse::cButtonRight );
				else 
					result = mouse.fDragDelta( tMouse::cButtonMiddle );
			}
			break;
		default:
			sigassert(!"Invalid type");
			break;
		}

		if ( mFlipX ) result.x *= -1;
		if ( mFlipY ) result.y *= -1;

		return result;
	}
}}
