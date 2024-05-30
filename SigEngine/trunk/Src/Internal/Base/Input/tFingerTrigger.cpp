#include "BasePch.hpp"
#include "tFingerTrigger.hpp"
#include "StringUtil.hpp"
#include "tTouchImplementationDetails.hpp"
#include "Input/tMouse.hpp"

namespace Sig { namespace Input {
	using namespace ImplDetails;

	namespace
	{
		tMouse::tButton gFauxFingerButtons[] = { tMouse::cButtonLeft, tMouse::cButtonRight, tMouse::cButtonMiddle };
	}

	tFingerTrigger::tFingerTrigger()
		: mType((tType)-1)
	{
	}

	b32 tFingerTrigger::fLoadFromString( const char* s )
	{
		using namespace StringUtil;

		if( !s || !*s )
		{
			// empty cell
			mType = cTypeDown;
			return true;
		}

		s = fReadUntilNonSpace( s );
		b32 down=false, held=false, up=false, toggle=false, dragStart=false, anyFinger=false;

		while( *s != '\0' )
		{
			b32 foundFlag
				=	fCheckFlag( s, "Down", down )
				||	fCheckFlag( s, "Held", held )
				||	fCheckFlag( s, "Up", up )
				||	fCheckFlag( s, "Release", up )
				||	fCheckFlag( s, "Toggle", toggle )
				||	fCheckFlag( s, "Drag", dragStart )
				||	fCheckFlag( s, "Any", anyFinger )
				;
			if( foundFlag )
				continue;

			if( fIsAnyOf( "0123456789", *s ) )
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
		if( down		) ++typeFlags, mType = cTypeDown;
		if( held		) ++typeFlags, mType = cTypeHeld;
		if( up			) ++typeFlags, mType = cTypeUp;
		if( toggle		) ++typeFlags, mType = cTypeToggle;
		if( dragStart	) ++typeFlags, mType = cTypeDragThreshold;
		sigassert( typeFlags == 1 );
		if( typeFlags != 1 )
			return false;

		if( anyFinger )
			mFingerFilter.fSetAll( true );

		return true;
	}

	void tFingerTrigger::fUpdate( b32& value, const tTouch& touch ) const
	{
		for( u32 i=0 ; i<tTouch::cFingerCount ; ++i )
		{
			if( !mFingerFilter.fGetBit(i+1) )
				continue;

			const tTouch::tFinger finger = (tTouch::tFinger)i;
			switch( mType ) {
			case cTypeDown:
				if( touch.fFingerDown(finger) )
				{
					value = true;
					return;
				}
				break;
			case cTypeHeld:
				if( touch.fFingerHeld(finger) )
				{
					value = true;
					return;
				}
				break;
			case cTypeUp:
				if( touch.fFingerUp(finger) )
				{
					value = true;
					return;
				}
				break;
			case cTypeDragThreshold:
				if( touch.fFingerDragging(finger) )
				{
					value = true;
					return;
				}
				break;
			case cTypeToggle:
				if( touch.fFingerDown(finger) )
				{
					value = !value;
					return;
				}
				break;
			default:
				sigassert(!"Invalid tFingerTrigger::mType in fEvaluate");
				break;
			}
		}

		if( mType != cTypeToggle )
			value = false;
	}

	void tFingerTrigger::fUpdate( b32& value, const tMouse& mouse ) const
	{
		u32 heldFingerCount = 0;
		for( u32 i=0 ; i<array_length(gFauxFingerButtons) ; ++i )
		{
			const tMouse::tButton faux = gFauxFingerButtons[i];
			if( mouse.fButtonHeld(faux) || mouse.fButtonUp(faux) )
				++heldFingerCount;
		}

		for( u32 i=0 ; i<array_length(gFauxFingerButtons) && i<mFingerFilter.cNumBits; ++i )
		{
			if( !mFingerFilter.fGetBit(i+1) )
				continue;

			const tMouse::tButton button = gFauxFingerButtons[i];
			switch( mType ) 
			{
			case cTypeDown:
				if( mouse.fButtonDown(button) )
				{
					value = true;
					return;
				}
				break;
			case cTypeHeld:
				if( mouse.fButtonHeld(button) )
				{
					value = true;
					return;
				}
				break;
			case cTypeUp:
				if( mouse.fButtonUp(button) )
				{
					value = true;
					return;
				}
				break;
			case cTypeDragThreshold:
				if( mouse.fDragDelta(button).fLengthSquared() > cDragStartThreshold*cDragStartThreshold )
				{
					value = true;
					return;
				}
				break;
			case cTypeToggle:
				if( mouse.fButtonDown(button) )
				{
					value = !value;
					return;
				}
				break;
			default:
				sigassert(!"Invalid tFingerTrigger::mType in fEvaluate");
				break;
			}
		}

		if( mType != cTypeToggle )
			value = false;
	}
}}
