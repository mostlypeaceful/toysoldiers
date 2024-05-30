#include "BasePch.hpp"


#ifdef sig_devmenu
#include "tUser.hpp"


namespace Sig
{
	std::string tDevVar<bool>::fIthItemValueString( u32 i ) const
	{
		return ( mValue ? "true" : "false" );
	}

	void tDevVar<bool>::fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) ||
			user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonRShoulder ) )
		{
			mValue = !mValue;
		}
	}

	void tDevVar<bool>::fSetFromString( const std::string& v )
	{
		mValue = ( ( v == "true" ) || ( v != "0" ) );
	}

	void tDevVar<bool>::fSetFromVector( const Math::tVec4f& v )
	{
		mValue = ( v.x ? true : false );
	}

	std::string tDevVar<tStringPtr>::fIthItemValueString( u32 i ) const
	{
		return std::string( mValue.fCStr( ) );
	}

	void tDevVar<tStringPtr>::fOnHighlighted( u32 ithItem, tUser & user, f32 absTime, f32 dt )
	{
		
	}

	void tDevVar<tStringPtr>::fSetFromString( const std::string& v )
	{
		mValue = tStringPtr( v );
	}

	std::string tDevVarVectorBase::fIthItemName( u32 i ) const
	{
		std::stringstream ss;
		ss << fShortName( ) << '.';
		if( i >= 4 )
			ss << std::setfill( '0' ) << std::setw( 4 ) << i;
		else
		{
			switch( i )
			{
			case 0: ss << ( mRgbaStyleName ? 'r' : 'x' ); break;
			case 1: ss << ( mRgbaStyleName ? 'g' : 'y' ); break;
			case 2: ss << ( mRgbaStyleName ? 'b' : 'z' ); break;
			case 3: ss << ( mRgbaStyleName ? 'a' : 'w' ); break;
			}
		}
		return ss.str( );
	}


	tDevCallback::tDevCallback( const char* path, const std::string& initialValueText, const tFunction& function )
		: tDevVarBase( path )
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

	void tDevCallback::fOnHighlighted( u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		tEventType event = cEventTypeNA;
		
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) ) event = cEventTypeDecrement;
		else if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonRShoulder ) ) event = cEventTypeIncrement;

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

}

#else

// Fixes no object linker warning
void tDevVarTypesCPP_NoObjFix( ) { }

#endif//sig_devmenu
