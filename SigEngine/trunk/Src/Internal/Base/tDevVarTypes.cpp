#include "BasePch.hpp"


#ifdef sig_devmenu
#include "tUser.hpp"
#include "Gui/tColorPicker.hpp"


namespace Sig
{
	const char* tDevVarVectorBase::gRGBAStrings[4] = { "R", "G", "B", "A" };
	const char* tDevVarVectorBase::gXYZWStrings[4] = { "X", "Y", "Z", "W" };

	std::string tDevVar<bool>::fIthItemValueString( u32 i ) const
	{
		return ( mValue ? "true" : "false" );
	}

	void tDevVar<bool>::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) ||
			user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonRShoulder ) ||
            ( user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonLeft  ) && user.fRawKeyboard( ).fNumFramesHeld( Input::tKeyboard::cButtonLeft ) < 2 ) ||
            ( user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonRight ) && user.fRawKeyboard( ).fNumFramesHeld( Input::tKeyboard::cButtonRight ) < 2 ) )
		{
			mValue = !mValue;
		}
	}

	void tDevVar<bool>::fSetFromVector( const Math::tVec4f& v )
	{
		mValue = ( v.x ? true : false );
	}

	void tDevVar<bool>::fSetItemValue( u32 index, const std::string & value )
	{
		if( index == 0 )
		{
			if( value == "true" )
				mValue = true;
			else if( value == "false" )
				mValue = false;
		}
	}

	std::string tDevVar<tStringPtr>::fIthItemValueString( u32 i ) const
	{
		return std::string( mValue.fCStr( ) );
	}

	void tDevVar<tStringPtr>::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser & user, f32 absTime, f32 dt )
	{
		
	}

	void tDevVar<tStringPtr>::fSetFromString( const std::string& v )
	{
		mValue = tStringPtr( v );
	}

	void tDevVar<tStringPtr>::fSetItemValue( u32 index, const std::string & value ) 
	{
		if( index == 0 )
			mValue = tStringPtr( value );
	}

	std::string tDevVarVectorBase::fIthItemName( u32 i ) const
	{
		std::stringstream ss;
		ss << fShortName( ) << ": ";
		if( i >= 4 )
			ss << std::setfill( '0' ) << std::setw( 4 ) << i;
		else
		{
			ss << mNameStrings[ i ];
		}
		return ss.str( );
	}


	tDevCallback::tDevCallback( const char* path, const std::string& initialValueText, const tFunction& function )
		: tDevMenuItem( path )
		, mValueText( initialValueText )
		, mFunction( function )
	{
		sigassert( !mFunction.fNull( ) );
		fFindAndApplyIniOverride( );
	}

	std::string tDevCallback::fIthItemValueString( u32 i ) const
	{
		return mValueText;
	}

	void tDevCallback::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		tEventType event = cEventTypeNA;
		
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) ) event = cEventTypeDecrement;
		else if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonRShoulder ) ) event = cEventTypeIncrement;
        else if( user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonLeft ) ) event = cEventTypeDecrement;
        else if( user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonRight ) ) event = cEventTypeIncrement;

		if( event != cEventTypeNA )
		{
			tArgs args( &user, event );
			args.mValueText = mValueText;

			mFunction( args );

			mValueText = args.mValueText;
		}
	}

	void tDevCallback::fSetFromString( const std::string& v ) 
	{
		tArgs args( NULL, cEventTypeSetValue );
		args.mValueText = v;

		mFunction( args );

		mValueText = args.mValueText;
	}


	namespace
	{
		void fUpdateColorValue( tDevMenuBase& menu, tUser& user, f32 dt, Math::tVec4f& value, b32 doAlpha, const Math::tVec4f& min, const Math::tVec4f& max )
		{
			const Input::tGamepad& gamepad = user.fRawGamepad( );

			// step color
			{
				Math::tVec3f rgb = value.fXYZ( );	
				const f32 cColorSpeed = 0.2f;
				const f32 cSatSpeed = 0.5f;
				Math::tVec3f delta( gamepad.fLeftStick( ).x * cColorSpeed, gamepad.fRightStick( ).y * cSatSpeed, gamepad.fLeftStick( ).y * cColorSpeed ); //hsv;
				delta *= dt;

				Gui::tColorPicker::fIncrementColor( rgb, delta, min.x, max.x );
				value = Math::tVec4f( rgb, value.w );
			}

			if( doAlpha )
			{
				// step alpha
				f32 deltaStep = 0.f;
				if( user.fRawGamepad( ).fButtonHeld( Input::tGamepad::cButtonLShoulder ) ) deltaStep -= 1.f;
				if( user.fRawGamepad( ).fButtonHeld( Input::tGamepad::cButtonRShoulder ) ) deltaStep += 1.f;

				const f32 cAlphaSpeed = 0.2f;
				value.w += deltaStep * cAlphaSpeed * dt;
				value.w = fClamp( value.w, min.w, max.w );
			}
		}
	}

	void tColorDevVar::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		Math::tVec4f value = *this;
		fUpdateColorValue( menu, user, dt, value, mShowAlpha, mMin, mMax );
		fSetFromVector( value );
		mV3Cache = value.fXYZ( );
		menu.fColorPicker( )->fSetColor( value, mMin.x, mMax.x );
		menu.fColorPicker( )->fDraw( *user.fScreen( ), mShowAlpha );
	}

	void tColorDevVarPtr3::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		Math::tVec4f value( *this, 0 );
		fUpdateColorValue( menu, user, dt, value, mShowAlpha, Math::tVec4f( mMin, 0 ), Math::tVec4f( mMax, 1 ) );
		fSetFromVector( value );
		menu.fColorPicker( )->fSetColor( value, mMin.x, mMax.x );
		menu.fColorPicker( )->fDraw( *user.fScreen( ), mShowAlpha );
	}

	void tColorDevVarPtr4::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		Math::tVec4f value = *this;
		fUpdateColorValue( menu, user, dt, value, mShowAlpha, mMin, mMax );
		fSetFromVector( value );
		menu.fColorPicker( )->fSetColor( value, mMin.x, mMax.x );
		menu.fColorPicker( )->fDraw( *user.fScreen( ), mShowAlpha );
	}

}

#else

// Fixes no object linker warning
void tDevVarTypesCPP_NoObjFix( ) { }

#endif//sig_devmenu
