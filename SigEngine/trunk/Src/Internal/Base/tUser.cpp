#include "BasePch.hpp"
#include "tUser.hpp"
#include "Fui.hpp"

namespace Sig
{
	static_assert( sizeof( tPlatformUserId ) == sizeof( u64 ) );

	devvar( bool, DevMenu_Enabled, true );


	//------------------------------------------------------------------------------
	// tUserData
	//------------------------------------------------------------------------------
	u32	tUserData::fDataSize( ) const
	{
		if( mType == cTypeS32 )
			return sizeof( mS32Data );
		if( mType == cTypeS64 )
			return sizeof( mS64Data );
		if( mType == cTypeF32 )
			return sizeof( mF32Data );
		if( mType == cTypeF64 )
			return sizeof( mF64Data );
		if( mType == cTypeUnicode )
			return mUnicodeData.mCharCount * sizeof( wchar_t );
		if( mType == cTypeBinary )
			return mBinaryData.mByteCount;

		// mType == cTypeNull
		return 0;
	}

	//------------------------------------------------------------------------------
	const void * tUserData::fDataPtr( )const
	{
		if( mType == cTypeS32 )
			return &mS32Data;
		if( mType == cTypeS64 )
			return &mS64Data;
		if( mType == cTypeF32 )
			return &mF32Data;
		if( mType == cTypeF64 )
			return &mF64Data;
		if( mType == cTypeUnicode )
			return mUnicodeData.mChars;
		if( mType == cTypeBinary )
			return mBinaryData.mBytes;

		// mType == cTypeNull
		return NULL;
	}

	//------------------------------------------------------------------------------
	std::string tUserData::fDataToString( ) const
	{
		std::stringstream ss;

		if( mType == tUserData::cTypeS32 )
			ss << tLocalizedString::fLocalizeNumber( mS32Data ).fToCString( );
		else if( mType == tUserData::cTypeS64 )
			ss << tLocalizedString::fLocalizeNumber( mS64Data ).fToCString( );
		else if( mType == tUserData::cTypeF64 )
			ss << mF64Data;
		else if( mType == tUserData::cTypeF32 )
			ss << mF32Data;
		else if( mType == tUserData::cTypeBinary )
			ss << std::string( (char *)mBinaryData.mBytes, mBinaryData.mByteCount );
		else if( mType == tUserData::cTypeNull )
			ss << "{NULL}";
		else if ( mType == tUserData::cTypeUnicode )
			ss << tLocalizedString( std::wstring( mUnicodeData.mChars, mUnicodeData.mCharCount ) ).fToCString( );
		else
			ss << "Unimplemented data type";

		return ss.str( );
	}

	//------------------------------------------------------------------------------
	tLocalizedString tUserData::fDataToTimeString( ) const
	{
		if( mType == tUserData::cTypeS32 )
			return tLocalizedString::fConstructTimeString( (f32)mS32Data, false );
		else if( mType == tUserData::cTypeS64 )
			return tLocalizedString::fConstructTimeString( (f32)mS64Data, false );
		else if( mType == tUserData::cTypeF64 )
			return tLocalizedString::fConstructTimeString( (f32)mF64Data, false );
		else if( mType == tUserData::cTypeF32 )
			return tLocalizedString::fConstructTimeString( mF32Data, false );
		else
			return tLocalizedString::fFromCString( "??" );

		return tLocalizedString::fNullString( );
	}

	//------------------------------------------------------------------------------
	b32 tUserData::operator==( const tUserData & other ) const
	{
		return  mType == other.mType &&
			 	fDataSize( ) == other.fDataSize( ) &&
				!fMemCmp( fDataPtr( ), other.fDataPtr( ), fDataSize( ) );
	}

	//------------------------------------------------------------------------------
	b32 tUserData::operator!=( const tUserData & other ) const
	{
		return mType != other.mType ||
				fDataSize( ) != other.fDataSize( ) ||
				fMemCmp( fDataPtr( ), other.fDataPtr( ), fDataSize( ) );
	}


	//------------------------------------------------------------------------------
	// tUserInfo
	//------------------------------------------------------------------------------
	tUserInfo::tUserInfo( )
		: mUserId( tUser::cInvalidUserId )
		, mOfflineId( tUser::cInvalidUserId )
		, mAddOnFlags( 0 )
	{
		mGamerTag.fFromCStr( L"" );
	}

	//------------------------------------------------------------------------------
	void tUserInfo::fReset( b32 resetAddOnFlags )
	{
		mUserId = tUser::cInvalidUserId;
		mOfflineId = tUser::cInvalidUserId;
		mGamerTag.fFromCStr( L"" );

		if( resetAddOnFlags )
		{
			mAddOnFlags = 0;
			mAddOnLicenses = 0;
		}
	}

	//------------------------------------------------------------------------------
	// tUser
	//------------------------------------------------------------------------------
	tUser::tUser( u64 windowHandle, u32 localHwIndex )
		: mGameRefCount( 0 )
		, mLocalHwIndex( localHwIndex )
		, mWarningInputFilter( 0 )
	{
		sigassert( localHwIndex < tUser::cMaxLocalUsers && "Invalid local hw index for local user" );
		fStartupMouse( windowHandle );
		fRefreshPlatformId( );
		mInputFilterStack.fPushBack( 0 );
	}

	tUser::tUser( const tUserInfo & info )
		: mUserInfo( info )
		, mGameRefCount( 0 )
		, mLocalHwIndex( ~0 )
		, mWarningInputFilter( 0 )
	{
		mInputFilterStack.fPushBack( 0 );
	}

	tUser::~tUser( )
	{
	}

	b32	tUser::fHasPendingOperations( ) const
	{
		const u32 opCount = mOperations.fCount( );
		for( u32 o = 0; o < opCount; ++o )
		{
			if( !mOperations[ o ]->fIsComplete( ) )
				return true;
		}

		return false;
	}

	void tUser::fWaitForOperations( )
	{
		while( mOperations.fCount( ) )
			mOperations.fPopBack( )->fWaitToComplete( );
	}

	void tUser::fPollInputDevices( f32 dt )
	{
		// capture state of hardware devices
		if( mLocalHwIndex != ~0 )
			mGamepad.fCaptureState( mLocalHwIndex, dt );

		mMouse.fCaptureState( dt );
		mKeyboard.fCaptureState( dt );

		fStepDevMenu( dt );
	}
	
	b32 tUser::fIsTopFilter( u32 filterID ) const
	{
		if( Fui::fIgnoreFilteredInput( ) )
			return false;

		if( mWarningInputFilter )
			return filterID == cWarningInputFilterLevel;

		return filterID == mInputFilterStack.fBack( ); 
	}

	u32	 tUser::fIncInputFilterLevel( ) 
	{ 
		u32 id = fNextInputFilterID( );
		mInputFilterStack.fPushBack( id );
		return id; 
	}

	u32  tUser::fIncWarningInputFilterLevel( )
	{
		++mWarningInputFilter;
		return cWarningInputFilterLevel;
	}

	void  tUser::fDecInputFilterLevel( u32 filterID ) 
	{
		if( filterID == 0 )
		{
			sigassert( filterID != 0 && "Zero is an invalid id when decrementing filter level" );
			return;
		}

		if( filterID != cWarningInputFilterLevel )
		{
			b32 found = mInputFilterStack.fFindAndEraseOrdered( filterID );
			sigassert( found );
		}
		else
		{
			sigassert( mWarningInputFilter );
			--mWarningInputFilter;
		}
	}

	const Input::tGamepad& tUser::fFilteredGamepad( u32 filterID ) const
	{
		return fIsTopFilter( filterID ) ? mGamepad : Input::tGamepad::cNullGamepad;
	}

	const Input::tMouse& tUser::fFilteredMouse( u32 filterID ) const
	{
		return fIsTopFilter( filterID ) ? mMouse : Input::tMouse::cNullMouse;
	}

	const Input::tKeyboard& tUser::fFilteredKeyboard( u32 filterID ) const
	{
		return fIsTopFilter( filterID ) ? mKeyboard : Input::tKeyboard::cNullKeyboard;
	}

	Input::tGamepad* tUser::fFilteredGamepadFromScript( u32 filterID )
	{
		return (Input::tGamepad *)&fFilteredGamepad( filterID );
	}

	Input::tMouse* tUser::fFilteredMouseFromScript( u32 filterID )
	{
		return (Input::tMouse*)&fFilteredMouse( filterID );
	}

	Input::tKeyboard* tUser::fFilteredKeyboardFromScript( u32 filterID )
	{
		return (Input::tKeyboard*)&fFilteredKeyboard( filterID );
	}

	const Input::tGamepad& tUser::fRawGamepad( ) const
	{
		return mGamepad;
	}

	const Input::tMouse& tUser::fRawMouse( ) const
	{
		return mMouse;
	}

	const Input::tKeyboard& tUser::fRawKeyboard( ) const
	{
		return mKeyboard;
	}

	Input::tGamepad* tUser::fRawGamepadFromScript( u32 level )
	{
		return &mGamepad;
	}

	Input::tMouse* tUser::fRawMouseFromScript( u32 level )
	{
		return &mMouse;
	}

	Input::tKeyboard* tUser::fRawKeyboardFromScript( u32 level )
	{
		return &mKeyboard;
	}

	void tUser::fStartupMouse( u64 windowHandle )
	{
		mMouse.fStartup( ( Input::tMouse::tGenericWindowHandle )windowHandle );
	}

	void tUser::fStepDevMenu( f32 dt )
	{
#ifdef sig_devmenu
		// handle dev menu toggling
		if( mViewport /*&& mLocalHwIndex != ~0*/ )
		{
			const b32 selectHeld = mGamepad.fButtonHeld( Input::tGamepad::cButtonSelect ) || mKeyboard.fButtonHeld( Input::tKeyboard::cButtonC );
			const b32 startDown = mGamepad.fButtonDown( Input::tGamepad::cButtonStart ) || mKeyboard.fButtonDown( Input::tKeyboard::cButtonEscape );
			const b32 toggleDevMenu = selectHeld && startDown && DevMenu_Enabled;

			if( toggleDevMenu )
			{
				if( mDevMenu.fIsActive( ) )
				{
					mDevMenu.fDeactivate( *this );
				}
				else
				{
					const Math::tRect safeRect = fComputeViewportSafeRect( );
					mDevMenu.fActivate( *this, safeRect.fTopLeft( ), safeRect.fHeight( ) );
				}
			}

			mDevMenu.fOnTick( *this, dt );
		}
#endif
	}

	Math::tRayf tUser::fComputePickingRay( const Math::tVec2u &screenPt ) const
	{
		Math::tRect rect = fComputeViewportRect( );
		Math::tVec2u corner( (u32)rect.mL, (u32)rect.mT );
		Math::tVec2f res = rect.fWidthHeight( );
		Math::tVec2u r( (u32)res.x, (u32)res.y );
		return fViewport( )->fRenderCamera( ).fComputePickRay( screenPt - corner, r );
	}

	Math::tRayf tUser::fComputePickingRay( const Math::tVec2f &screenPt ) const
	{
		Math::tVec2u p( (u32)screenPt.x, (u32)screenPt.y );
		return fComputePickingRay( p );
	}

	Math::tRect tUser::fComputeScreenSafeRect( ) const
	{
		return mScreen->fComputeSafeRect( );
	}

	Math::tRect tUser::fComputeViewportSafeRect( ) const
	{
		return mViewport->fComputeSafeRect( );

		//const Math::tVec2u safeEdge = mScreen->fComputeGuiSafeEdge( );
		//const Math::tRect vpClipBox = mViewport->fClipBox( );

		//const f32 bbWidth  = ( f32 )mScreen->fCreateOpts( ).mBackBufferWidth;
		//const f32 bbHeight = ( f32 )mScreen->fCreateOpts( ).mBackBufferHeight;
		//f32 l = fRound<f32>( bbWidth * vpClipBox.mL );
		//f32 t = fRound<f32>( bbHeight * vpClipBox.mT );
		//f32 r = fRound<f32>( bbWidth * vpClipBox.mR );
		//f32 b = fRound<f32>( bbHeight * vpClipBox.mB );

		//l += safeEdge.x * ( 1.f - vpClipBox.mL );
		//t += safeEdge.y * ( 1.f - vpClipBox.mT );
		//r -= safeEdge.x * ( vpClipBox.mR - 0.f );
		//b -= safeEdge.y * ( vpClipBox.mB - 0.f );

		//return Math::tRect( t, l, b, r );
	}

	Math::tRect tUser::fComputeViewportRect( ) const
	{
		return mViewport->fComputeRect( );

		//const f32 bbWidth  = ( f32 )mScreen->fCreateOpts( ).mBackBufferWidth;
		//const f32 bbHeight = ( f32 )mScreen->fCreateOpts( ).mBackBufferHeight;
		//const f32 l = fRound<f32>( bbWidth * mViewport->fClipBox( ).mL );
		//const f32 t = fRound<f32>( bbHeight * mViewport->fClipBox( ).mT );
		//const f32 r = fRound<f32>( bbWidth * mViewport->fClipBox( ).mR );
		//const f32 b = fRound<f32>( bbHeight * mViewport->fClipBox( ).mB );

		//return Math::tRect( t, l, b, r );
	}

	f32 tUser::fAspectRatio( ) const
	{
		const Math::tRect vpRect = fComputeViewportRect( );
		return vpRect.fAspectRatio( );
	}

	void tUser::fPlatformSettingsChanged( )
	{
		log_line( 0, "user " << this << " settings have changed" );
		mUserInfo.fRefreshGamerTag( mLocalHwIndex );
	}

	void tUser::fAddGamepadState( const Input::tGamepad::tStateData& data )
	{
		mGamepad.fPutStateData( data );
	}
	
	const Input::tGamepad::tStateData& tUser::fGetGamepadState( )
	{
		return mGamepad.fGetStateData( );
	}

	//------------------------------------------------------------------------------
	tUser::tOperationPtr tUser::fNewOperation( )
	{
		tOperationPtr result;

		s32 o = mOperations.fCount( ) - 1;

		// Find a valid result
		for( ; o >= 0; --o )
		{
			if( mOperations[ o ]->fIsComplete( ) )
			{
				result = mOperations[ o-- ];
				result->fReset( );
				break;
			}
		}

		// If we found a result then run over whats left and free other completed operations
		if( result )
		{
			for( ; o >= 0; --o )
			{
				if( mOperations[ o ]->fIsComplete( ) )
					mOperations.fErase( o );
			}
		}
		else
		{
			result.fReset( NEW tOperation( ) );
			mOperations.fPushBack( result );
		}

		return result;
	}

	bool tUser::fAnyUserButtonHeldFromScript( u32 buttonMask )
	{
		for( u32 u = 0; u < cMaxLocalUsers; ++u )
		{
			Input::tGamepad pad;
			pad.fCaptureState( u );
			if( pad.fButtonHeld( buttonMask ) )
				return true;
		}

		return false;
	}

	tKeyboardUIOpPtr tUser::fShowKeyboardUI( const tLocalizedString& title, const tLocalizedString& defaultText ) const
	{
		tKeyboardUIOpPtr uiOp = tKeyboardUIOpPtr( NEW tKeyboardUIOp( ) );
		uiOp->fCreate( mLocalHwIndex, title, defaultText );
		return uiOp;
	}

	tKeyboardUIOpPtr tUser::fShowNamingKeyboardUI( const tLocalizedString& title, const tLocalizedString& defaultText ) const
	{
		tKeyboardUIOpPtr uiOp = tKeyboardUIOpPtr( NEW tKeyboardUIOp( ) );
		uiOp->fCreateForName( mLocalHwIndex, title, defaultText );
		return uiOp;
	}

	void tUser::fShowGamerCardUIFromScript( tScript64 * id )
	{
		if( id )
			fShowGamerCardUI( ( tPlatformUserId )id->fGet( ) );
	}

	//------------------------------------------------------------------------------
	// tUserArray
	//------------------------------------------------------------------------------
	u32 tUserArray::fFirstSignedInOrValid( ) const
	{
		u32 randUser = ~0;
		const u32 count = fCount( );
		for( u32 u = 0; u < count; ++u )
		{
			const tUserPtr & user = operator[]( u );
			if( !user )
				continue;

			if( randUser == ~0 )
				randUser = u;

			if( !user->fSignedIn( ) )
				continue;

			return u;
		}

		log_assert( fInBounds<u32>( randUser, 0, count ), "No valid users signed in" );
		return randUser;
	}
}


namespace Sig
{
	namespace
	{
		u32 fDataToInt( tUserData* data )
		{
			if( data->fType( ) == tUserData::cTypeS64 )
				return u32( data->fGetS64( ) );
			else
				return 0;
		}
	}

	void tUser::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tUserData, Sqrat::DefaultAllocator<tUserData>> classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("IsSet"),				&tUserData::fIsSet)
				.Func(_SC("ToString"),			&tUserData::fDataToString)
				.Func(_SC("ToFloat"),			&tUserData::fGetF32)
				.Func(_SC("ToTimeString"),		&tUserData::fDataToTimeString)
				.GlobalFunc(_SC("ToInt"),		&fDataToInt)
				;
			vm.fRootTable( ).Bind( _SC("User"), classDesc );
		}

		Sqrat::Class<tUser,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop("GamerTag", &tUser::fGamerTag)
			.Prop("UserId", &tUser::fUserIdFromScript)
			.Prop("IsGuest", &tUser::fIsGuest)
			.Prop("IsOnlineEnabled", &tUser::fIsOnlineEnabled)
			.Prop("IsLocal", &tUser::fIsLocalFromScript)
			.Prop("IsInActiveParty", &tUser::fIsInActivePartyFromScript )
			.Prop("InputFilterLevel", &tUser::fInputFilterLevel )
			.Func("IncInputFilterLevel", &tUser::fIncInputFilterLevel)
			.Func("IncWarningInputFilterLevel", &tUser::fIncWarningInputFilterLevel)
			.Func("DecInputFilterLevel", &tUser::fDecInputFilterLevel)
			.Func("FilteredGamepad", &tUser::fFilteredGamepadFromScript)
			.Func("FilteredMouse", &tUser::fFilteredMouseFromScript)
			.Func("FilteredKeyboard", &tUser::fFilteredKeyboardFromScript)
			.Func("RawGamepad", &tUser::fRawGamepadFromScript)
			.Func("RawMouse", &tUser::fRawMouseFromScript)
			.Func("RawKeyboard", &tUser::fRawKeyboardFromScript)
			.Func("ComputeScreenSafeRect", &tUser::fComputeScreenSafeRect)
			.Func("ComputeViewportRect", &tUser::fComputeViewportRect)
			.Func("ComputeViewportSafeRect", &tUser::fComputeViewportSafeRect)
			.Prop("SignedIn", &tUser::fSignedIn)
			.Prop("SignedInOnline", &tUser::fSignedInOnline)
			.Func("HasPrivilege", &tUser::fHasPrivilegeFromScript)
			.Prop("AddOnsInstalled", &tUser::fAddOnsInstalled)
			.Func("ShowSignInUI", &tUser::fShowSignInUI)
			.Func("ShowAchievementsUI", &tUser::fShowAchievementsUI)
			.Func("ShowMarketplaceUI", &tUser::fShowMarketplaceUI)
			.Func("ShowGamerCardUI", &tUser::fShowGamerCardUIFromScript)
			.Func("ShowFriendsUI", &tUser::fShowFriendsUI)
			.Func("ShowInviteUI", &tUser::fShowInviteUI)
			.Func("ShowCommunitySessionsUI", &tUser::fShowCommunitySessionsUI)
			.Prop("LocalHwIndex", &tUser::fLocalHwIndex)
			.Prop("ViewportIndex", &tUser::fViewportIndex)
			.Prop("IsViewportVirtual", &tUser::fIsViewportVirtual)
			.StaticFunc("AnyUserButtonHeld", &tUser::fAnyUserButtonHeldFromScript)
			.Prop("YAxisInvertedDefault", &tUser::fYAxisInvertedDefault)
			.Prop("SouthpawDefault", &tUser::fSouthpawDefault)
			;
		vm.fRootTable( ).Bind( _SC("User"), classDesc );

		vm.fConstTable( ).Const( "PRIVILEGE_MULTIPLAYER", ( int )tUser::cPrivilegeMultiplayer );
	}

}
