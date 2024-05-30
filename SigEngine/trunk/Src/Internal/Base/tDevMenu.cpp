#include "BasePch.hpp"

#ifdef sig_devmenu
#include "tUser.hpp"
#include "tResource.hpp"
#include "tGameAppBase.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "Gfx/tIndexBufferVRamSlice.hpp"
#include "Gui/tColoredQuad.hpp"
#include "Gui/tText.hpp"
#include "Gui/tFont.hpp"
#include "Gui/tColorPicker.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	namespace
	{
		struct tSortPagesAlphabetically
		{
			inline b32 operator()( const tDevVarDepot::tPagePtr& a, const tDevVarDepot::tPagePtr& b ) const
			{
				return StringUtil::fStricmp( a->mName.c_str( ), b->mName.c_str( ) ) < 0;
			}
		};
		struct tSortVarsAlphabetically
		{
			inline b32 operator()( const tDevMenuItem* a, const tDevMenuItem* b ) const
			{
				return StringUtil::fStricmp( a->fShortName( ).c_str( ), b->fShortName( ).c_str( ) ) < 0;
			}
		};

		void fDumpDevMenu( tDevCallback::tArgs& args )
		{
			args.mUser->fDevMenu( ).fLogValuesToFile( );
		}
	}

	void tDevMenuItem::fSplitPath( const char* path, std::string& shortName, std::string& folder )
	{
		const char* lastDot = strrchr( path, '_' );

		if( lastDot )
		{
			folder = std::string( path, lastDot );
			shortName = lastDot + 1;
		}
		else
		{
			folder = ""; // goes at the root
			shortName = path;
		}

		// each variable must have a name
		sigassert( shortName.length( ) > 0 );
	}

	tDevMenuItem::tDevMenuItem( const char* path, u32 numItems )
		: mNumItems( numItems )
		, mNumDisplayItems( numItems )
		, mFullPath( path )
		, mInputTiming( 10 )
	{
		fSplitPath( path, mShortName, mDirectory );
		tDevVarDepot::fInstance( ).fInsert( *this );
	}

	tDevMenuItem::~tDevMenuItem( )
	{
		tDevVarDepot::fInstance( ).fRemove( *this );
	}

	void tDevMenuItem::fSetNumDisplayItems( u32 num ) 
	{ 
		tDevVarDepot::fInstance( ).fRemove( *this );
		mNumDisplayItems = num;
		tDevVarDepot::fInstance( ).fInsert( *this ); 
	}

	u32 tDevMenuItem::fFindItemIndex( const std::string & itemName )
	{
		for( u32 i = 0; i < mNumItems; ++i )
		{
			if( itemName == fIthItemName( i ) )
				return i;
		}

		return ~0;
	}

	void tDevMenuItem::fFindAndApplyIniOverride( )
	{
		// see if any devmenu ini objects have been registered;
		// look through them to see if there's an override specified for this variable
		for( u32 i = 0; i < tDevVarDepot::fInstance( ).mInis.fCount( ); ++i )
		{
			const tDevMenuIni::tEntry* find = tDevVarDepot::fInstance( ).mInis[ i ]->fFindEntry( mFullPath );
			if( find )
			{
				if( !find->mStringValue.fNull( ) )
				{
					std::string str( find->mStringValue.fCStr( ) );
					fSetFromString( str );
				}
				else
					fSetFromVector( find->mValue );
			}
		}
	}

	f32 tDevMenuItem::fComputeNumericDelta( f32 value, f32 inc, tUser& user, f32 absTime, f32 dt )
	{
		f32 deltaStep = 0.f;
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonLShoulder ) || user.fRawGamepad( ).fButtonHeldRepeat( Input::tGamepad::cButtonLShoulder, 10 ) 
            || user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonLeft ) || user.fRawKeyboard( ).fNumFramesHeld( Input::tKeyboard::cButtonLeft ) > 10 )
			deltaStep -= 1.f;
		if( user.fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonRShoulder ) || user.fRawGamepad( ).fButtonHeldRepeat( Input::tGamepad::cButtonRShoulder, 10 )
            || user.fRawKeyboard( ).fButtonDown( Input::tKeyboard::cButtonRight ) || user.fRawKeyboard( ).fNumFramesHeld( Input::tKeyboard::cButtonRight ) > 10 )
			deltaStep += 1.f;

		tClientData& timingData = mInputTiming[ &user ];
		if( deltaStep != 0.f && fEqual( timingData.mLastTime + dt, absTime, 1.f/5.f ) )
			timingData.mHeldTime += dt;
		else
			timingData.mHeldTime  = 0.f;
		timingData.mLastTime = absTime; // track time

		//log_line( 0, "timingData.mHeldTime = " << timingData.mHeldTime );

		const u32 timeBasedIncMultiplier = ( u32 )( 1.f + Math::fPow( fMin( 32.f, fRoundDown<f32>( 2.0f * timingData.mHeldTime ) ), 3.f ) );
		const f32 deltaMagnitude = timeBasedIncMultiplier * inc;

		//log_line( 0, "timeBasedIncMultiplier = " << timeBasedIncMultiplier );
		//log_line( 0, "deltaMagnitude = " << deltaMagnitude );

		return deltaMagnitude * deltaStep;
	}

	void tDevVarDepot::tPage::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		if( ithItem < mPages.fCount( ) )
			return; // a page is highlighted

		ithItem -= mPages.fCount( );

		u32 numItemsCounted = 0;

		for( u32 i = 0; i < mVars.fCount( ); ++i )
		{
			if( ithItem >= numItemsCounted && ithItem < numItemsCounted + mVars[ i ]->fNumDisplayItems( ) )
			{
				mVars[ i ]->fOnHighlighted( menu, ithItem - numItemsCounted, user, absTime, dt );
				break;
			}

			numItemsCounted += mVars[ i ]->fNumDisplayItems( );
		}
	}

	std::string tDevVarDepot::tPage::fFullPathString( ) const
	{
		tGrowableArray<std::string> subNames;

		subNames.fPushBack( mName );
		for( const tPage* parent = mParent; parent; parent = parent->mParent )
			subNames.fPushBack( parent->mName );

		std::stringstream ss;
		for( u32 i = subNames.fCount( ); i > 0; --i )
		{
			if( subNames[ i - 1 ].length( ) > 0 )
			{
				ss << subNames[ i - 1 ];
				if( i > 1 )
					ss << "\\";
			}
			else
				ss << "$:\\";
		}

		return ss.str( );
	}

	std::string tDevVarDepot::tPage::fSubPagesString( u32 scrollCount ) const
	{
		std::stringstream ss;

		for( u32 i = scrollCount; i < mPages.fCount( ); ++i )
			ss << ".\\" << mPages[ i ]->mName << "\\...\n";

		return ss.str( );
	}

	std::string tDevVarDepot::tPage::fVarItemsString( u32 scrollCount ) const
	{
		std::stringstream ss;

		u32 numItemsCounted = 0;
		for( u32 i = 0; i < mVars.fCount( ); ++i )
		{
			const u32 numItems = mVars[ i ]->fNumDisplayItems( );
			for( u32 j = 0; j < numItems; ++j )
			{
				if( numItemsCounted >= scrollCount )
					ss << mVars[ i ]->fIthItemName( j ) << "\n";
				++numItemsCounted;
			}
		}

		return ss.str( );
	}

	std::string tDevVarDepot::tPage::fVarValuesString( u32 scrollCount ) const
	{
		std::stringstream ss;

		u32 numItemsCounted = 0;
		for( u32 i = 0; i < mVars.fCount( ); ++i )
		{
			const u32 numItems = mVars[ i ]->fNumDisplayItems( );
			for( u32 j = 0; j < numItems; ++j )
			{
				if( numItemsCounted >= scrollCount )
					ss << mVars[ i ]->fIthItemValueString( j ) << "\n";
				++numItemsCounted;
			}
		}

		return ss.str( );
	}

	tDevVarDepot::tDevVarDepot( )
		: mRoot( NEW tPage( 0, "" ) )
	{
	}

	tDevVarDepot::~tDevVarDepot( )
	{
	}

	tDevMenuItem* tDevVarDepot::fFindVar( const char* path )
	{
		tDevVarDepot::tPage * page = NULL;
		u32 idx = -1;

		if( fFindVarPos( path, page, idx ) )
			return page->mVars[ idx ];

		return NULL;
	}

	b32 tDevVarDepot::fFindVarPos( const char* path, tDevVarDepot::tPage*& outPage, u32& outPos )
	{
		std::string shortName, folder;
		tDevMenuItem::fSplitPath( path, shortName, folder );

		// search for entry in depot and update value if found
		tDevVarDepot::tPagePtr p = fFindPage( folder, false );
		if( p.fNull( ) )
			return false;

		for( u32 i = 0; i < p->mVars.fCount( ); ++i )
		{
			if( StringUtil::fStricmp( p->mVars[ i ]->fShortName( ).c_str( ), shortName.c_str( ) ) == 0 )
			{
				outPage = p.fGetRawPtr();
				outPos = i;
				return true;
			}
		}

		return false;
	}

	tDevVarDepot::tPage * tDevVarDepot::fFindPage( const char * folder )
	{
		// search for entry in depot and update value if found
		tDevVarDepot::tPagePtr p = fFindPage( folder, false );
		return p.fGetRawPtr( );
	}

	void tDevVarDepot::fInsert( tDevMenuItem& var )
	{
		tPagePtr p = fFindPage( var.fDirectory( ), true );
		sigassert( !p.fNull( ) );

		p->mVars.fPushBack( &var );
		p->mNumVarItems += var.fNumDisplayItems( );
		std::sort( p->mVars.fBegin( ), p->mVars.fEnd( ), tSortVarsAlphabetically( ) );
	}

	void tDevVarDepot::fRemove( tDevMenuItem& var )
	{
		tPagePtr p = fFindPage( var.fDirectory( ), false );

		if( p.fNull( ) )
			return; // doesn't live in any pages

		tDevMenuItem* const* find = p->mVars.fFind( &var );
		if( find )
		{
			p->mNumVarItems -= var.fNumDisplayItems( );
			p->mVars.fEraseOrdered( fPtrDiff( find, p->mVars.fBegin( ) ) );

			if( p->mVars.fCount( ) == 0 )
			{
				// remove page from parent
				if( p->mParent )
				{
					tPagePtr parent( p->mParent );
					p->mParent = 0;
					tPagePtr* locationInParentArray = parent->mPages.fFind( p );
					sigassert( locationInParentArray );
					parent->mPages.fEraseOrdered( fPtrDiff( locationInParentArray, parent->mPages.fBegin( ) ) );
				}
			}
		}
	}

	tDevVarDepot::tPagePtr tDevVarDepot::fFindPage( const std::string& path, b32 createIfNotFound )
	{
		// ensure the root page exists
		sigassert( !mRoot.fNull( ) );

		tGrowableArray<std::string> pageNames;
		StringUtil::fSplit( pageNames, path.c_str( ), "_" );

		tPagePtr parent = mRoot;

		for( u32 ipage = 0; ipage < pageNames.fCount( ); ++ipage )
		{
			b32 found = false;

			for( u32 i = 0; i < parent->mPages.fCount( ); ++i )
			{
				if( StringUtil::fStricmp( parent->mPages[ i ]->mName.c_str( ), pageNames[ ipage ].c_str( ) ) == 0 )
				{
					parent = parent->mPages[ i ];
					found = true;
					break;
				}
			}

			if( !found )
			{
				if( createIfNotFound )
				{
					tPagePtr newPage( NEW tPage( parent.fGetRawPtr( ), pageNames[ ipage ] ) );
					parent->mPages.fPushBack( newPage );
					std::sort( parent->mPages.fBegin( ), parent->mPages.fEnd( ), tSortPagesAlphabetically( ) );
					parent = newPage;
				}
				else
				{
					return tPagePtr( );
				}
			}
		}

		return parent;
	}

	devrgba_clamp( DevMenu_Colors_CurrentFolder, Math::tVec4f( 1.0f, 0.4f, 0.2f, 1.0f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_SubFolder, Math::tVec4f( 0.9f, 0.8f, 0.8f, 1.0f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_EntryName, Math::tVec4f( 1.0f, 1.0f, 1.0f, 1.0f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_EntryValue, Math::tVec4f( 1.0f, 1.0f, 0.0f, 1.0f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_Background, Math::tVec4f( 0.2f, 0.2f, 0.2f, 0.7f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_PathSeparator, Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.5f ), 0.f, 1.f );
	devrgba_clamp( DevMenu_Colors_HighlightBar, Math::tVec4f( 0.0f, 0.0f, 0.25f, 0.5f ), 0.f, 1.f );
	devcb( DevMenu_Dump, "Dump", make_delegate_cfn( tDevCallback::tFunction, fDumpDevMenu ) );

	class base_export tDevMenuImpl : public tDevMenuBase
	{
	private:

		b32										mActive;
		f32										mAbsTime;
		f32										mLineHeight;
		f32										mMenuWidth;
		f32										mLeftMargin;
		f32										mTopMargin;
		f32										mBodyTextStartY;
		f32										mBodyTextHeight;
		f32										mHeight;
		Math::tVec2f							mOrigin;
		tResourcePtr							mSmallFont;

		Gui::tText								mPathText;
		Gui::tText								mDirNamesText;
		Gui::tText								mVarNamesText;
		Gui::tText								mVarValuesText;
		Gui::tColoredQuad						mBackground;
		Gui::tColoredQuad						mHighlightBar;
		Gui::tColoredQuad						mPathSeparator;
		Gui::tColorPicker						mColorPicker;

		tDevVarDepot::tPagePtr					mCurPage;
		s32										mCurItem;
		s32										mScrollCount;

	public:
		tDevMenuImpl( )
			: mActive( false )
			, mAbsTime( 0.f )
			, mLineHeight( 0.f )
			, mMenuWidth( 300.f )
			, mLeftMargin( 10.f )
			, mTopMargin( 10.f )
			, mBodyTextStartY( 0.f )
			, mBodyTextHeight( 0.f )
			, mHeight( 0.f )
			, mOrigin( Math::tVec2f::cZeroVector )
			, mCurItem( -1 )
			, mScrollCount( 0 )
		{
		}

		void fInitDevMenu( const tResourcePtr& smallFont )
		{
			mSmallFont = smallFont;

			mLineHeight = smallFont->fCast<Gui::tFont>( )->mDesc.mLineHeight;

			mPathText.fSetFont( smallFont );
			mDirNamesText.fSetFont( smallFont );
			mVarNamesText.fSetFont( smallFont );
			mVarValuesText.fSetFont( smallFont );
			mColorPicker.fSetFont( smallFont );

			mBackground.fSetRect( Math::tRect( ) );
			mPathSeparator.fSetRect( Math::tRect( ) );
			mHighlightBar.fSetRect( Math::tRect( ) );

			mCurPage = tDevVarDepot::fInstance( ).fRoot( );
			mCurItem = -1;
		}

		void fSetDevMenuStart( const std::string& start )
		{
			if( start.length() <= 0 )
				return;

			tDevVarDepot::tPage * outPage = NULL;
			u32 outPos = -1;
			if( tDevVarDepot::fInstance( ).fFindVarPos( start.c_str(), outPage, outPos ) )
			{
				mCurPage.fReset( outPage );
				mCurItem = outPos+1;
			}
		}

		b32 fIsActive( ) const
		{
			return mActive;
		}
		
		virtual Gui::tColorPicker* fColorPicker( ) { return &mColorPicker; }

		void fEnsureValidPageAndItem( )
		{
			if( mCurPage.fNull( ) || mCurPage->fIsOrphaned( ) )
				mCurPage = tDevVarDepot::fInstance( ).fRoot( );

			sigassert( !mCurPage.fNull( ) );

			if( mCurItem < 0 || mCurItem >= ( s32 )mCurPage->fNumTotalItems( ) )
				mCurItem = mCurPage->mPages.fCount( );
		}

		void fActivate( const Math::tVec2f& origin, const f32 height )
		{
			if( mSmallFont.fNull( ) )
				return;

			mActive = true;
			mOrigin = origin;
			mHeight = height;
			mBodyTextStartY = mOrigin.y + mTopMargin + mLineHeight + mTopMargin;
			mBodyTextHeight = mHeight - mBodyTextStartY;


			// set path text toward top left of area
			const Math::tVec3f pathTextPos = Math::tVec3f( origin.x + mLeftMargin, origin.y + mTopMargin, 0.0f );
			mPathText.fSetPosition( pathTextPos );

			// set background to fill the specified space, with top left at origin
			mBackground.fSetPosition( Math::tVec3f( origin, 0.01f ) );

			// set the path separator bar just underneath the path text
			mPathSeparator.fSetPosition( Math::tVec3f( origin.x, pathTextPos.y + mLineHeight + mTopMargin / 2.f, 0.005f ) );	

			mBackground.fSetRect( Math::tVec2f( mMenuWidth, height ) );
			mPathSeparator.fSetRect( Math::tVec2f( mMenuWidth, 2.f ) );
			mHighlightBar.fSetRect( Math::tVec2f( mMenuWidth, mLineHeight ) );

			mColorPicker.fSetPosition( Math::tVec3f( origin.x + mMenuWidth + 5, pathTextPos.y + mLineHeight + mTopMargin / 2.f, 0.005f ) );
		}

		void fDeactivate( )
		{
			mActive = false;
		}

		void fChangePage( const tDevVarDepot::tPagePtr& newPage )
		{
			mCurPage->mLastSelectedItem[ this ] = mCurItem;
			mCurPage = newPage;
			mCurItem = mCurPage->mLastSelectedItem[ this ];

			fEnsureValidPageAndItem( );
		}

		void fPlacePageTextAndHighlightBar( )
		{
			fEnsureValidPageAndItem( );

			const s32 numItemsCanFit = fRoundDown<s32>( mBodyTextHeight / mLineHeight );
			mScrollCount = fMax( 0, mCurItem - numItemsCanFit );

			// set directory names text a little under path text
			const Math::tVec3f dirNamesTextPos = Math::tVec3f( mOrigin.x + mLeftMargin, mBodyTextStartY, 0.f );
			mDirNamesText.fSetPosition( dirNamesTextPos );

			// set variable names text a directly under directory text
			const s32 effectiveNumPages = fMax( 0, ( s32 )mCurPage->mPages.fCount( ) - mScrollCount );
			const Math::tVec3f varNamesTextPos = Math::tVec3f( dirNamesTextPos.x, dirNamesTextPos.y + effectiveNumPages * mLineHeight, 0.f );
			mVarNamesText.fSetPosition( varNamesTextPos );

			// set variables values to same position as names, but right aligned
			const Math::tVec3f varValuesTextPos = Math::tVec3f( mOrigin.x - mLeftMargin, varNamesTextPos.y, 0.f );
			mVarValuesText.fSetPosition( varValuesTextPos );

			// set the highlight cursor at the current position
			mHighlightBar.fSetPosition( Math::tVec3f( mOrigin.x, dirNamesTextPos.y + ( mCurItem - mScrollCount ) * mLineHeight, 0.005f ) );
		}

		void fAssignUIColors( )
		{
			mPathText.fSetRgbaTint( DevMenu_Colors_CurrentFolder );
			mDirNamesText.fSetRgbaTint( DevMenu_Colors_SubFolder );
			mVarNamesText.fSetRgbaTint( DevMenu_Colors_EntryName );
			mVarValuesText.fSetRgbaTint( DevMenu_Colors_EntryValue );
			mBackground.fSetRgbaTint( DevMenu_Colors_Background );
			mPathSeparator.fSetRgbaTint( DevMenu_Colors_PathSeparator );
			mHighlightBar.fSetRgbaTint( DevMenu_Colors_HighlightBar );
		}

		void fOnTick( tUser& user, f32 dt )
		{
			if( !fIsActive( ) ) return;

			mAbsTime += dt;

			const Input::tGamepad& gamepad = user.fRawGamepad( );
            const Input::tKeyboard& keyboard = user.fRawKeyboard( );

			fEnsureValidPageAndItem( );

			if( gamepad.fButtonDown( Input::tGamepad::cButtonDPadDown ) || gamepad.fButtonHeldRepeat( Input::tGamepad::cButtonDPadDown, 10 )
                || keyboard.fButtonDown( Input::tKeyboard::cButtonDown ) || keyboard.fNumFramesHeld( Input::tKeyboard::cButtonDown ) > 10 )
				mCurItem += 1;
			if( gamepad.fButtonDown( Input::tGamepad::cButtonDPadUp ) || gamepad.fButtonHeldRepeat( Input::tGamepad::cButtonDPadUp, 10 )
                || keyboard.fButtonDown( Input::tKeyboard::cButtonUp ) || keyboard.fNumFramesHeld( Input::tKeyboard::cButtonUp ) > 10 )
				mCurItem -= 1;

			if( mCurItem < 0 )
				mCurItem = mCurPage->fNumTotalItems( ) - 1;
			else if( mCurItem >= ( s32 )mCurPage->fNumTotalItems( ) )
				mCurItem = 0;

			if( ( gamepad.fButtonDown( Input::tGamepad::cButtonA ) || keyboard.fButtonDown( Input::tKeyboard::cButtonEnter ) ) && ( u32 )mCurItem < mCurPage->mPages.fCount( ) )
				fChangePage( mCurPage->mPages[ mCurItem ] );
			if( ( gamepad.fButtonDown( Input::tGamepad::cButtonB ) || keyboard.fButtonDown( Input::tKeyboard::cButtonEscape ) ) && mCurPage->mParent )
				fChangePage( tDevVarDepot::tPagePtr( mCurPage->mParent ) );

			if( mCurItem >= ( s32 )mCurPage->mPages.fCount( ) && mCurItem < ( s32 )mCurPage->fNumTotalItems( ) )
				mCurPage->fOnHighlighted( *this, mCurItem, user, mAbsTime, dt );

			if( ( gamepad.fButtonDown( Input::tGamepad::cButtonStart ) || keyboard.fButtonDown( Input::tKeyboard::cButtonEnter ) )
                && !gamepad.fButtonHeld( Input::tGamepad::cButtonSelect ) && !keyboard.fButtonHeld( Input::tKeyboard::cButtonEscape ) )
			{
				tGameAppBase* gameApp = tApplication::fInstance( ).fDynamicCast< tGameAppBase >( );
				gameApp->fPause( !gameApp->fPaused( ) );
			}

			// it's possible that the menu has been de-activated by a callback function,
			// so check before we enqueue anything for rendering and exit out if so
			if( !mActive )
				return;

			fPlacePageTextAndHighlightBar( );
			fAssignUIColors( );

			user.fScreen( )->fAddScreenSpaceDrawCall( mBackground.fDrawCall( ) );
			user.fScreen( )->fAddScreenSpaceDrawCall( mHighlightBar.fDrawCall( ) );
			user.fScreen( )->fAddScreenSpaceDrawCall( mPathSeparator.fDrawCall( ) );

			const std::string path = mCurPage->fFullPathString( );
			if( path.length( ) > 0 )
			{
				mPathText.fBake( path.c_str( ), 0, Gui::tText::cAlignLeft );
				user.fScreen( )->fAddScreenSpaceDrawCall( mPathText.fDrawCall( ) );
			}

			const std::string subPagesString = mCurPage->fSubPagesString( mScrollCount );
			if( subPagesString.length( ) && subPagesString[0] )
			{
				mDirNamesText.fBakeBox( Math::cInfinity, subPagesString.c_str( ), 0, Gui::tText::cAlignLeft );
				user.fScreen( )->fAddScreenSpaceDrawCall( mDirNamesText.fDrawCall( ) );
			}

			// we still print text until the pages are gone
			const u32 scrollCountVars = ( u32 )fMax<s32>( ( s32 )mScrollCount - mCurPage->mPages.fCount( ), 0 );

			const std::string varItemsString = mCurPage->fVarItemsString( scrollCountVars );
			if( varItemsString.length( ) && varItemsString[0] )
			{
				mVarNamesText.fBakeBox( Math::cInfinity, varItemsString.c_str( ), 0, Gui::tText::cAlignLeft );
				user.fScreen( )->fAddScreenSpaceDrawCall( mVarNamesText.fDrawCall( ) );
			}

			const std::string varValuesString = mCurPage->fVarValuesString( scrollCountVars );
			if( varValuesString.length( ) && varValuesString[0] )
			{
				mVarValuesText.fBakeBox( mMenuWidth, varValuesString.c_str( ), 0, Gui::tText::cAlignRight );
				user.fScreen( )->fAddScreenSpaceDrawCall( mVarValuesText.fDrawCall( ) );
			}
		}

		void fCaptureVarInfo( const char * folder, std::stringstream & ss, b32 recursive)
		{
			tDevVarDepot::tPage * page = tDevVarDepot::fInstance( ).fFindPage( folder );
			if( !page )
				return;
			
			std::string shortName, parentFolder;
			tDevMenuItem::fSplitPath( folder, shortName, parentFolder );
			fGetPageVars( parentFolder.c_str( ), *page, ss, recursive );
		}

		void fLoadVarInfo( std::stringstream & ss )
		{
			tGrowableArray< std::string > equalSplit; equalSplit.fReserve( 2 );
			tGrowableArray< std::string > itemSplit; itemSplit.fReserve( 2 );

			while( 1 )
			{
				std::string line;
				std::getline( ss, line );

				// Split on the equals sign
				equalSplit.fSetCount( 0 );
				StringUtil::fSplit( equalSplit, line.c_str( ), "=" );

				// This means we're done
				if( equalSplit.fCount( ) != 2 )
					break;

				equalSplit[ 0 ] = StringUtil::fEatWhiteSpace( equalSplit[ 0 ] );
				equalSplit[ 1 ] = StringUtil::fEatWhiteSpace( equalSplit[ 1 ] );

				// Get the folder and the name
				std::string shortName, ignored;
				tDevMenuItem::fSplitPath( equalSplit[ 0 ].c_str( ), shortName, ignored );

				// Get the name and the item
				itemSplit.fSetCount( 0 );
				StringUtil::fSplit( itemSplit, equalSplit[ 0 ].c_str( ), "." );

				// Find the var
				tDevMenuItem * var = tDevVarDepot::fInstance( ).fFindVar( itemSplit.fFront( ).c_str( ) );
				if( !var )
				{
					log_warning( "Variable " << itemSplit.fFront( ) << " could not be found" );
					continue;
				}

				// Find the item index
				u32 index = var->fFindItemIndex( shortName );
				if( index == ~0 )
				{
					log_warning( "Variable " << var->fFullPath( ) << " has no item index for " << shortName );
					continue;
				}

				var->fSetItemValue( index, equalSplit[ 1 ] );
			}
		}

		void fLogValuesRecursive( const tDevVarDepot::tPage& page, std::stringstream& output )
		{
			for( u32 i = 0; i < page.mPages.fCount( ); ++i )
				fLogValuesRecursive( *page.mPages[ i ], output );

			for( u32 i = 0; i < page.mVars.fCount( ) ; ++i )
			{
				output << page.mVars[ i ]->fFullPath( ) << "\t";
				for( u32 v = 0; v < page.mVars[ i ]->fNumDisplayItems( ); ++v )
				{
					output << page.mVars[ i ]->fIthItemValueString( v );
					if( v != page.mVars[ i ]->fNumDisplayItems( ) - 1 )
						output << ", ";
				}
				output << "\n";
			}
		}

		void fLogValuesToFile( )
		{
#ifdef target_xbox360
			const tFilePathPtr cDumpPath( "game:\\dev_menu_dump.txt" );
#else
			//log_warning( "Could not dump dev menu, no dev menu dump path set on this platform" );
			//return;

			// todo, find the most reasonably way to generate dump paths on the target machine.
			const tFilePathPtr cDumpPath( "c:\\SignalStudios\\Temp\\dev_menu_dump.txt" );
#endif

			std::stringstream ss;
			fLogValuesRecursive( *tDevVarDepot::fInstance( ).fRoot( ), ss );
			tFileWriter writer( cDumpPath, false );

			std::string str = ss.str( );
			writer( str.c_str( ), str.length( ) );
		}

		void fGetPageVars( const char * parentFolder, tDevVarDepot::tPage & page, std::stringstream & ss, b32 recursive )
		{
			std::string folderName;
			{
				std::stringstream temp;
				temp << parentFolder << "_" << page.mName;
				folderName = temp.str( );
			}

			const u32 varCount = page.mVars.fCount( );
			for( u32 v = 0; v < varCount; ++v )
			{
				const u32 itemCount = page.mVars[ v ]->fNumDisplayItems( );
				for( u32 i = 0; i < itemCount; ++i )
				{
					ss << folderName << "_" << page.mVars[ v ]->fIthItemName( i ) << " = " << page.mVars[ v ]->fIthItemValueString( i ) << std::endl;
				}
			}

			if( recursive )
			{
				const u32 pageCount = page.mPages.fCount( );
				for( u32 p = 0; p < pageCount; ++p )
				{
					if( page.mPages[ p ] )
						fGetPageVars( folderName.c_str( ), *page.mPages[ p ], ss, true );
				}
			}
		}
	};

	tDevMenu::tDevMenu( )
		: mImpl( NEW tDevMenuImpl )
		, mGamePadFilter( 0 )
	{
	}

	tDevMenu::~tDevMenu( )
	{
		delete mImpl;
	}

	void tDevMenu::fInitDevMenu( const tResourcePtr& smallFont )
	{
		mImpl->fInitDevMenu( smallFont );
	}

	void tDevMenu::fSetDevMenuStart( const std::string& start )
	{
		mImpl->fSetDevMenuStart( start );
	}

	b32  tDevMenu::fIsActive( ) const
	{
		return mImpl->fIsActive( );
	}

	void tDevMenu::fActivate( tUser& user, const Math::tVec2f& origin, const f32 height )
	{
		mImpl->fActivate( origin, height );
		mGamePadFilter = user.fIncInputFilterLevel( );
	}

	void tDevMenu::fDeactivate( tUser& user )
	{
		mImpl->fDeactivate( );
		user.fDecInputFilterLevel( mGamePadFilter );
	}

	void tDevMenu::fOnTick( tUser& user, f32 dt )
	{
		mImpl->fOnTick( user, dt );
	}

	void tDevMenu::fCaptureVarInfo( const char * folder, std::stringstream & ss, b32 recursive )
	{
		mImpl->fCaptureVarInfo( folder, ss, recursive );
	}

	void tDevMenu::fLoadVarInfo( std::stringstream & ss )
	{
		mImpl->fLoadVarInfo( ss );
	}

	void tDevMenu::fLogValuesToFile( )
	{
		return mImpl->fLogValuesToFile( );
	}
}

#else

// Fixes no object linker warning
void tDevMenuCPP_NoObjFix( ) { }

#endif//sig_devmenu
