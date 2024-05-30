#ifndef __tUser__
#define __tUser__
#include "tLocalizationFile.hpp"
#include "Gfx/tScreen.hpp"
#include "Input/tGamepad.hpp"
#include "Input/tMouse.hpp"
#include "Input/tKeyboard.hpp"
#include "tSync.hpp"
#include "Scripts/tScript64.hpp"
#include "XtlUtil.hpp"

namespace Sig
{
#ifdef platform_xbox360
	typedef XUID tPlatformUserId;
	enum { cSettingsMaxSize = XPROFILE_SETTING_MAX_SIZE }; //1000
#else
	typedef u64 tPlatformUserId;
	enum { cSettingsMaxSize = 0 };
#endif

#ifdef platform_xbox360
	class tKeyboardUIOp : public XtlUtil::tOverlappedOp, public tRefCounter
	{	
	public:

		b32 fSuccess( );
		const wchar_t* fResultText( ) const { return mResultText.fCStr( ); }

		b32 fCreate( u32 userIndex, const tLocalizedString& title, const tLocalizedString& defaultText );
		b32 fCreateForName( u32 userIndex, const tLocalizedString& title, const tLocalizedString& defaultText );

	private:
		
		b32 fCreate( u32 fontType, u32 userIndex, const tLocalizedString& title, const tLocalizedString& defaultText );

	private:
		tLocalizedString mTitle;
		tLocalizedString mDefaultText;
		tLocalizedString mResultText;
	};
#else
	class tKeyboardUIOp : public tRefCounter
	{	
	public:

		b32 fIsComplete( ) { return true; }
		b32 fSuccess( ) { return false; }
		const wchar_t* fResultText( ) const { return L""; }

		b32 fCreate( u32 userIndex, const tLocalizedString& title, const tLocalizedString& defaultText ) { return false; }
		b32 fCreateForName( u32 userIndex, const tLocalizedString& title, const tLocalizedString& defaultText ) { return false; }
	};
#endif
	typedef tRefCounterPtr< tKeyboardUIOp > tKeyboardUIOpPtr;

	enum tSettingsIndex
	{
		cSettingsIndex0,
		cSettingsIndex1,
		cSettingsIndex2,
		cTotalSettings
	};

	// This is abstracted by platform.
	class tUserProfileSaveBase
	{
	public:
		virtual ~tUserProfileSaveBase( ) { }
		virtual b32 fSaveDataMT( byte* data, u32 size ) = 0;
	};

	///
	/// \class tUserData
	/// \brief 
	class tUserData
	{
	public:

		static const u32 cTypeS32; // Stored
		static const u32 cTypeS64; // Stored
		static const u32 cTypeF32; // Stored
		static const u32 cTypeF64; // Stored
		static const u32 cTypeUnicode; // Pointer
		static const u32 cTypeBinary;  // Pointer
		static const u32 cTypeNull; // Nothing

		tUserData( ) : mType( cTypeNull ) { }

		u32 fType( ) const { return mType; }
		b32 fIsSet( ) const { return mType != cTypeNull; }

		u32	fDataSize( ) const;
		const void * fDataPtr( ) const;
		std::string fDataToString( ) const;
		tLocalizedString fDataToTimeString( ) const;

		//------------------------------------------------------------------------------
		// Sets
		//------------------------------------------------------------------------------
		void fReset( ) { mType = cTypeNull; }
		void fSet( s32 data ) { mType = cTypeS32; mS32Data = data; }
		void fSet( s64 data ) { mType = cTypeS64; mS64Data = data; }
		void fSet( f32 data ) { mType = cTypeF32; mF32Data = data; }
		void fSet( f64 data ) { mType = cTypeF64; mF64Data = data; }
		void fSetString( const wchar_t * str, u32 charCount ) 
		{ 
			mType = cTypeUnicode; 
			mUnicodeData.mChars = (wchar_t *)str;
			mUnicodeData.mCharCount = charCount;
		}
		void fSetBinary( const void * data, u32 dataLength )
		{
			mType = cTypeBinary;
			mBinaryData.mBytes = (void *)data;
			mBinaryData.mByteCount = dataLength;
		}

		void fSetPlatformSpecific( const void * platformUserData );

		//------------------------------------------------------------------------------
		// Gets
		//------------------------------------------------------------------------------
		s32 fGetS32( ) const { sigassert( mType == cTypeS32 ); return mS32Data; }
		s64 fGetS64( ) const { sigassert( mType == cTypeS64 ); return mS64Data; }
		f32 fGetF32( ) const { sigassert( mType == cTypeF32 ); return mF32Data; }
		f64 fGetF64( ) const { sigassert( mType == cTypeF64 ); return mF64Data; }
		const wchar_t * fGetString( ) const { sigassert( mType == cTypeUnicode ); return mUnicodeData.mChars; }
		const void * fGetBinary ( ) const { sigassert( mType == cTypeBinary ); return mBinaryData.mBytes; }

		void fGet( s32 & out ) const { sigassert( mType == cTypeS32 ); out = mS32Data; }
		void fGet( s64 & out ) const { sigassert( mType == cTypeS64 ); out = mS64Data; }
		void fGet( f32 & out ) const { sigassert( mType == cTypeF32 ); out = mF32Data; }
		void fGet( f64 & out ) const { sigassert( mType == cTypeF64 ); out = mF64Data; }
		void fGetString( const wchar_t *& out, u32 * charCount = NULL ) const
		{ 
			sigassert( mType == cTypeUnicode ); 
			out = mUnicodeData.mChars; 
			if( charCount )
				*charCount = mUnicodeData.mCharCount;
		}
		void fGetBinary( const void *& out, u32 & lenOut ) const
		{ 
			sigassert( mType == cTypeBinary ); 
			out = mBinaryData.mBytes; 
			lenOut = mBinaryData.mByteCount;
		}

		void fGetPlatformSpecific( void * platformSpecific ) const;

		// Operators
		b32 operator==( const tUserData & other ) const;
		b32 operator!=( const tUserData & other ) const;

	private:

		u32 mType;

		union
		{
			s32		mS32Data;
			s64		mS64Data;
			f32		mF32Data;
			f64		mF64Data;
			
			struct
			{
				wchar_t * mChars;
				u32		  mCharCount;
			}		mUnicodeData;

			struct
			{
				void *	 mBytes;
				u32		 mByteCount;
			}		mBinaryData;
		};
	};

	///
	/// \class tUserContext
	/// \brief 
	struct tUserContext
	{
		u32 mId;
		u32 mValue;

		tUserContext( u32 id = ~0, u32 value = ~0 )
			: mId( id ) , mValue( value ) { }
	};

	///
	/// \class tUserProperty
	/// \brief
	struct tUserProperty
	{
		u32 mId;
		tUserData mData;

		tUserProperty( u32 id = ~0 )
			: mId( id ) { }
	};

	///
	/// \class tUserInfo
	/// \brief 
	struct tUserInfo
	{
		tPlatformUserId mUserId;
		tPlatformUserId mOfflineId;

		u32 mAddOnFlags; // i.e., which DLCs does the user have installed
		u32 mAddOnLicenses; //whether or not the user has priveledges to access the add ons.
		tLocalizedString mGamerTag;

		tUserInfo( );
		tUserInfo( const tUserInfo & info )
			: mUserId( info.mUserId )
			, mOfflineId( info.mOfflineId )
			, mAddOnFlags( info.mAddOnFlags )
			, mAddOnLicenses( info.mAddOnLicenses )
			, mGamerTag( info.mGamerTag ) 
		{ }

		void fReset( b32 resetAddOnFlags = false );
		void fRefresh( u32 localHwIndex );
		void fRefreshGamerTag( u32 localHwIndex );

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mUserId );
			archive.fSaveLoad( mOfflineId );
			archive.fSaveLoad( mAddOnFlags );
			archive.fSaveLoad( mAddOnLicenses );
			archive.fSaveLoad( mGamerTag );
		}
	};

	///
	/// \brief Encapsulates the virtual presence of the real flesh-and-blood 
	/// human being interacting with the application.
	/// \see tApplication
	class base_export tUser : public tRefCounter
	{
	public:
		static const tPlatformUserId cInvalidUserId;

		// Unfortunately we can only use these as compile time constants if they are assigned in the header file
#ifdef platform_xbox360
		static const u32 cMaxLocalUsers = XUSER_MAX_COUNT;
		static const u32 cUserIndexNone = XUSER_INDEX_NONE;
		static const u32 cUserIndexAny = XUSER_INDEX_ANY;
		static const u32 cUserNameSize = XUSER_NAME_SIZE;
#else
		static const u32 cMaxLocalUsers = 4;
		static const u32 cUserIndexNone = 5;
		static const u32 cUserIndexAny = 6;
		static const u32 cUserNameSize = 16;
#endif

		enum tSignInState
		{
			cSignInStateNotSignedIn = 0,
			cSignInStateSignedInLocally,
			cSignInStateSignedInOnline
		};

	private:

		tDevMenu				mDevMenu;

		tUserInfo				mUserInfo;
		Gfx::tViewportPtr		mViewport;
		Gfx::tScreenPtr			mScreen;
		u32						mGameRefCount;
		u32						mLocalHwIndex;
		u32						mWarningInputFilter;

		// input
		tGrowableArray<u32>		mInputFilterStack;
		Input::tGamepad			mGamepad;
		Input::tMouse			mMouse;
		Input::tKeyboard		mKeyboard;

#ifdef platform_xbox360
		struct tOperation : public tRefCounter
		{
			tOperation( ) { fReset( ); }
			~tOperation( ) { sigcheckfail( fIsComplete( ), fWaitToComplete( ) ); }

			void fReset( ) { fZeroOut( mOverlapped ); }
			b32 fIsComplete( ) const { return XHasOverlappedIoCompleted( &mOverlapped ); }
			void fWaitToComplete( ) { DWORD ignored; (void)XGetOverlappedResult(&mOverlapped, &ignored, TRUE ); }

			tDynamicBuffer mData;
			XOVERLAPPED mOverlapped;
		};
#else
		struct tOperation : public tRefCounter 
		{ 
			void fReset( ) { }
			b32 fIsComplete( ) const { return true; }
			void fWaitToComplete( ) { }
		};
#endif

		typedef tRefCounterPtr<tOperation> tOperationPtr;
		tGrowableArray< tOperationPtr > mOperations;

	public:

		static const u32 cUserContextPresence;
		static const u32 cUserContextGameMode;
		static const u32 cUserContextGameType;
		static const u32 cUserContextGameTypeRanked;
		static const u32 cUserContextGameTypeStandard;

		static const u32 cPrivilegeMultiplayer;

		static const u32 cWarningInputFilterLevel = ~0;

	public:

		// Local user constructor
		explicit tUser( u64 windowHandle, u32 localHwIndex );

		// Remote user constructor
		explicit tUser( const tUserInfo & info );
		~tUser( );

		tDevMenu& fDevMenu( ) { return mDevMenu; }

		inline const tUserInfo & fUserInfo( ) const { return mUserInfo; }
		inline const tLocalizedString& fGamerTag( ) const { return mUserInfo.mGamerTag; }
		inline tPlatformUserId fPlatformId( ) const { return mUserInfo.mUserId; }
		inline tPlatformUserId fOfflinePlatformId( ) const { return mUserInfo.mOfflineId; }
		inline b32 fHasValidPlatformId( ) const { return mUserInfo.mUserId != cInvalidUserId; }

		///
		/// \brief Set user context information
		void	fSetContext( u32 id, u32 value );

		///
		/// \brief Set user property information
		void	fSetProperty( u32 id, const tUserData & value );

		///
		/// \brief Query if the user has pending operations
		b32		fHasPendingOperations( ) const;
		void    fWaitForOperations( );

		///
		/// \brief Should be called once per frame to update user input devices.
		/// \note Calls fStepDevMenu internally; if not calling fPollInputDevices, you may want to call that method explicitly.
		void	fPollInputDevices( f32 dt );

		// Manage input filtering
		u32		fInputFilterLevel( )
		{
			return mInputFilterStack.fBack( );
		}

		b32		fIsTopFilter( u32 filterID ) const;
		u32		fIncInputFilterLevel( );
		u32		fIncWarningInputFilterLevel( );
		void	fDecInputFilterLevel( u32 filterID );

		// Access filtered input devices.
		const Input::tGamepad&	fFilteredGamepad( u32 filterID ) const;
		const Input::tMouse&	fFilteredMouse( u32 filterID ) const;
		const Input::tKeyboard&	fFilteredKeyboard( u32 filterID ) const;
		Input::tGamepad*		fFilteredGamepadFromScript( u32 filterID );
		Input::tMouse*			fFilteredMouseFromScript( u32 filterID );
		Input::tKeyboard*		fFilteredKeyboardFromScript( u32 filterID );

		// Access raw input device access.
		const Input::tGamepad&	fRawGamepad( ) const;
		const Input::tMouse&	fRawMouse( ) const;
		const Input::tKeyboard&	fRawKeyboard( ) const;
		Input::tGamepad*		fRawGamepadFromScript( u32 level );
		Input::tMouse*			fRawMouseFromScript( u32 level );
		Input::tKeyboard*		fRawKeyboardFromScript( u32 level );

		
		///
		/// \brief start the mouse, useful if the user was created outside of input scope
		void fStartupMouse( u64 windowHandle );

		///
		/// \brief Updates the dev menu. If you're not calling fPollInputDevices, then you should call this explicitly.
		void fStepDevMenu( f32 dt );

		///
		/// \brief Query to find out if the user is local or networked
		b32 fIsLocal( ) const { return mLocalHwIndex != ~0; }

		///
		/// \brief Adds a game ref
		void fAddGameRef( ) { ++mGameRefCount; }
		
		///
		/// \brief Removes a game ref
		void fRemoveGameRef( ) { sigassert( mGameRefCount && "Mismatched game references" ); --mGameRefCount; }

		///
		/// \brief Query to find if the user is used by the game
		b32 fIsUsedByGame( ) const { return mGameRefCount != 0; }

		///
		/// \brief Set the currently bound graphics viewport.
		void fSetViewport( const Gfx::tScreenPtr& sp, const Gfx::tViewportPtr& vp ) { mScreen = sp; mViewport = vp; }

		/// \brief Get the viewport index
		u32 fViewportIndex( ) { sigassert( mViewport ); return mViewport->fViewportIndex( ); }

		/// \brief Check if the viewport is virtual
		b32 fIsViewportVirtual( ) { sigassert( mViewport ); return mViewport->fIsVirtual( ); }

		///
		/// \brief Get the currently bound graphics viewport (may be null).
		const Gfx::tViewportPtr& fViewport( ) const { return mViewport; }

		///
		/// \brief Get the currently bound screen (may be null).
		const Gfx::tScreenPtr& fScreen( ) const { return mScreen; }

		///
		/// \brief Converts a screen coordinate into a world ray through the current projection.
		Math::tRayf fComputePickingRay( const Math::tVec2u &screenPt ) const;
		Math::tRayf fComputePickingRay( const Math::tVec2f &screenPt ) const;

		///
		/// \brief Compute the "safe" rectangle for gui rendering corresponding to the entire screen.
		Math::tRect fComputeScreenSafeRect( ) const;

		///
		/// \brief Compute the "safe" rectangle for gui rendering corresponding to the viewport.
		Math::tRect fComputeViewportSafeRect( ) const;

		///
		/// \brief Compute the "un-safe" rectangle corresponding to the viewport.
		Math::tRect	fComputeViewportRect( ) const;

		///
		/// \brief Compute the aspect ratio for this user's viewport
		f32 fAspectRatio( ) const;

		///
		/// \brief Gets the local user index for this user.
		u32 fLocalHwIndex( ) const { return mLocalHwIndex; }

		///
		/// \brief Query the mask corresponding to which DLC/Add-On packs the user has installed
		u32 fAddOnsLicensed( ) const { return mUserInfo.mAddOnLicenses; }
		void fSetAddOnsLicensed( u32 addons ) { mUserInfo.mAddOnLicenses = addons; }
		u32 fAddOnsInstalled( ) const { return mUserInfo.mAddOnFlags; }
		void fSetAddOnsInstalled( u32 addons ) { mUserInfo.mAddOnFlags = addons; }

		///
		/// \brief Refreshes the platform-specific user ID, in response to some platform-specific event
		void fRefreshPlatformId( );

		///
		/// \brief Called when settings for the profile have changed
		void fPlatformSettingsChanged( );

		///
		/// \brief Adds state data to the gamepad buffer.  ie data from the network
		void fAddGamepadState( const Input::tGamepad::tStateData& data );

		///
		/// \brief Gets the most recent gamepad state data.
		const Input::tGamepad::tStateData& fGetGamepadState( );

		///
		/// \brief Find out if the user is signed in as a guest
		b32 fIsGuest( ) const;

		///
		/// \brief Find out if the user is signed in as a an Xbox LIVE enabled account
		b32 fIsOnlineEnabled( ) const;

		///
		/// \brief Find out if the user is "signed in" (platform-specific)
		b32 fSignedIn( ) const;

		///
		/// \brief Find out if the user is "signed in online" (platform-specific, i.e. signed in to LIVE)
		b32 fSignedInOnline( ) const;

		///
		/// \brief Returns the sign in state of the user
		tSignInState fSignInState( ) const;

		///
		/// \brief Determine if the user has the specified privilege
		b32 fHasPrivilege( u32 privilege ) const;

		///
		/// \brief Show platform-specific UI for allowing the user to sign in.
		void fShowSignInUI( bool onlineOnly ) const;

		///
		/// \brief Show platform-specific achievements UI
		void fShowAchievementsUI( ) const;

		///
		/// \brief Show marketplace UI - if 'offerId' is 0, it will list everything
		b32 fShowMarketplaceUI( u64 offerId ) const;

		///
		/// \brief Show the gamer card UI for the specified gamer
		void fShowGamerCardUI( tPlatformUserId toShow ) const;

		///
		/// \brief Show the platform-specific friends UI 
		void fShowFriendsUI( ) const;

		///
		/// \brief Show the platform-specific UI for inviting people which may vary based on
		/// platform-specific requirements. This centralizes those requirements.
		void fShowInviteUI( ) const;

		///
		/// \brief Show the platform-specific UI for available game sessions
		void fShowCommunitySessionsUI( ) const;

		///
		/// \brief Show platform-specific keyboard UI
		tKeyboardUIOpPtr fShowKeyboardUI( const tLocalizedString& title, const tLocalizedString& defaultText ) const;
		tKeyboardUIOpPtr fShowNamingKeyboardUI( const tLocalizedString& title, const tLocalizedString& defaultText ) const;

		///
		/// \brief Determine if the player is in an active platform-specific party
		b32 fIsInActiveParty( ) const;

		///
		/// \brief Reads the profile and writes it into data. Returns true if successful, false if not.
		b32 fReadProfile( tDynamicArray<byte>& dataOut );

		///
		/// \brief Writes data to the profile
		tUserProfileSaveBase* fCreateUserProfileSave( );

		///
		/// \brief Returns true if we're able to write
		b32 fCanWriteToProfile( );

		///
		/// \brief Reads the profile for the y-axis default setting
		b32 fYAxisInvertedDefault( );
		
		///
		/// \brief Reads the profile for the lefty default setting
		b32 fSouthpawDefault( );

		///
		/// \brief Award the avatar to the user
		b32 fAwardAvatar( u32 avatarId );

		///
		/// \brief Award the gamer picture
		b32 fAwardGamerPicture( u32 pictureId );

	public:

		static b32 fCheckFullLicenseFlag( );
		static b32 fIsGuest( u32 localHwIndex );
		static b32 fIsOnlineEnabled( u32 localHwIndex );
		static b32 fSignedIn( u32 localHwIndex );
		static b32 fIsMuted( u32 localHwIndex, tPlatformUserId test );
		static tPlatformUserId fGetUserId( u32 localHwIndex );

	private:

		bool fIsLocalFromScript( ) const { return fIsLocal( ) ? true : false; }
		bool fHasPrivilegeFromScript( u32 privilege ) const { return fHasPrivilege( privilege ) ? true : false; }
		bool fIsInActivePartyFromScript( ) const { return fIsInActiveParty( ) ? true : false; }
		tOperationPtr fNewOperation( );

		u32	fNextInputFilterID( )
		{
			u32 highest = 0;
			for( u32 i = 0; i < mInputFilterStack.fCount( ); ++i )
				highest = fMax( highest, mInputFilterStack[ i ] );
			return highest + 1;
		}

	private:
		static bool fAnyUserButtonHeldFromScript( u32 buttonMask );
		void fShowGamerCardUIFromScript( class tScript64 * id );
		tScript64 fUserIdFromScript( ) const { return tScript64( fPlatformId( ) ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tUser );


	struct tUserSigninInfo
	{
		tUserSigninInfo( ) 
			: mState( tUser::cSignInStateNotSignedIn )
			, mOnlineId( tUser::cInvalidUserId )
			, mOfflineId( tUser::cInvalidUserId ) { }

		tUser::tSignInState mState;
		tPlatformUserId mOnlineId;
		tPlatformUserId mOfflineId;
	};


	///
	/// \class tUserArray
	/// \brief An array of users
	class base_export tUserArray : public tGrowableArray< tUserPtr >
	{

	public:

		///
		/// \brief Returns the index of the first signed in user or the first non-null index or ~0
		u32 fFirstSignedInOrValid( ) const;

	};

}


#endif//__tUser__
