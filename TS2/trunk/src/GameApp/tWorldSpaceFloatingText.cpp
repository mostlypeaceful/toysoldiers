#include "GameAppPch.hpp"
#include "tWorldSpaceFloatingText.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tSync.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	tWorldSpaceFloatingText::tWorldSpaceFloatingText( const tUserPtr& user, u32 font, f32 zOffset ) 
		: mFloatSpeed( sync_rand( fFloatInRange( 2.f, 6.f ) ) )
		, mTimeToLive( 3.f )
		, mCurrentLifetime( 0.f )
		, mLimitedLife( true )
		, mFadeDelta( 0.f )
		, mScale( 1.f )
		, mFlyToScreenTimer( -Math::cInfinity )
		, mOverallScale( 0.f )
		, mUser( user )
		, mZOffset( zOffset )
	{
		std::stringstream ss;
		ss << "hover" << user->fViewportIndex( );
		fCreate( tGameApp::fInstance( ).fLocFont( font ), *user, tGameApp::fInstance( ).fHudLayer( ss.str( ) ).fToCanvasFrame( ) );
	}
	tWorldSpaceFloatingText::tWorldSpaceFloatingText( const tUserArray& userArray, u32 font, f32 zOffset ) 
		: mFloatSpeed( sync_rand( fFloatInRange( 2.f, 6.f ) ) )
		, mTimeToLive( 3.f )
		, mCurrentLifetime( 0.f )
		, mLimitedLife( true )
		, mFadeDelta( 0.f )
		, mScale( 1.f )
		, mFlyToScreenTimer( -Math::cInfinity )
		, mOverallScale( 0.f )
		, mUser( userArray[ 0 ] )
		, mZOffset( zOffset )
	{
		for( u32 i = 0; i < userArray.fCount( ); ++i )
		{
			std::stringstream ss;
			ss << "hover" << userArray[ i ]->fViewportIndex( );
			fAdd( tGameApp::fInstance( ).fLocFont( font ), *userArray[ i ], tGameApp::fInstance( ).fHudLayer( ss.str( ) ).fToCanvasFrame( ) );
		}
		//fCreate( tGameApp::fInstance( ).fLocFont( tGameApp::cFontFancyLarge ), userArray, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}
	tWorldSpaceFloatingText::~tWorldSpaceFloatingText( )
	{
	}
	void tWorldSpaceFloatingText::fOnSpawn( )
	{
		fRunListInsert( cRunListWorldSpaceUIST );		
	}
	void tWorldSpaceFloatingText::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListWorldSpaceUIST );			
		}
		else
		{
			fRunListInsert( cRunListWorldSpaceUIST );			
		}
	}
	void tWorldSpaceFloatingText::fWorldSpaceUIST( f32 dt )
	{
		mCurrentLifetime += dt;
		mFlyToScreenTimer += dt;

		if( mCurrentLifetime < 0.3333f )
			mOverallScale = mCurrentLifetime / 0.29f;	//give us a value just a bit over 1...
		if( mCurrentLifetime > ( mTimeToLive * 0.8f ) )
			mOverallScale -= 3.f * dt;

		mOverallScale = fClamp( mOverallScale, 0.f, 1.5f );

		Math::tVec3f pos = fObjectToWorld( ).fGetTranslation( );

		if( mFlyToScreenTimer > 0.f )
		{
			f32 wayThere = mFlyToScreenTimer / 1.f;		//get there in 1 second...
			// get cam pos
			const Gfx::tCamera& cam = mUser->fViewport( )->fRenderCamera( );
			Math::tVec3f idealPos = cam.fLocalToWorld( ).fGetTranslation( ) - ( cam.fLocalToWorld( ).fZAxis( ) * 5.f ) - ( cam.fLocalToWorld( ).fYAxis( ) * 20.f );
			if( tGameApp::fInstance( ).fGameMode( ).fIsSinglePlayer( ) )
				idealPos += cam.fLocalToWorld( ).fXAxis( ) * 20.f;

			pos = Math::fLerp( pos, idealPos, wayThere );
			mOverallScale = 1.3f - wayThere;
		}
		else
		{
			// just rise up...ya, tell that to the people
			pos.y += mFloatSpeed * dt;
		}

		pos.z += mZOffset;

		fMoveTo( pos );
		tWorldSpaceText::fWorldSpaceUIST( dt );

		if( mLimitedLife )
		{
			if( mCurrentLifetime > mTimeToLive )
				fFadeOut( );
		}

		b32 allTextsFadedOut = mLimitedLife;
		const tTextArray& text = fAccessText( );

		for( u32 i = 0; i < text.fCount( ); ++i )
		{		
			const f32 alpha = fClamp( text[ i ].mText->fAlpha( ) + mFadeDelta * dt, 0.f, 1.f );

			f32 scale = fMax( 0.f, text[ i ].mText->fZPos( ) - 0.9f ) / 0.1f;	
			scale = scale * scale;
			scale = Math::fLerp( 1.75f * mScale * alpha, 0.5f * mScale * alpha, scale );
		
			text[ i ].mText->fSetScale( Math::tVec2f( scale * mOverallScale ) );
			text[ i ].mText->fSetAlpha( alpha );

			if( alpha != 0.f || mFadeDelta >= 0.f )
				allTextsFadedOut = false;
		}

		if( allTextsFadedOut )
			fDelete( );
	}
	void tWorldSpaceFloatingText::fSetTint( const Math::tVec4f& rgba )
	{
		const tTextArray& textObj = fAccessText( );

		for( u32 i = 0; i < textObj.fCount( ); ++i )
			textObj[ i ].mText->fSetRgbaTint( rgba );
	}
	void tWorldSpaceFloatingText::fSetText( const char* text )
	{
		const tTextArray& textObj = fAccessText( );

		for( u32 i = 0; i < textObj.fCount( ); ++i )
		{
			textObj[ i ].mText->fBake( text, 0, Gui::tText::cAlignCenter );
			textObj[ i ].mText->fSetAlpha( 0.f );
		}
		fFadeIn( );
	}

	void tWorldSpaceFloatingText::fSetText( const tLocalizedString& locText )
	{
		const tTextArray& textObj = fAccessText( );

		for( u32 i = 0; i < textObj.fCount( ); ++i )
		{
			textObj[ i ].mText->fBake( locText, Gui::tText::cAlignCenter );
			textObj[ i ].mText->fSetAlpha( 0.f );
		}
		fFadeIn( );
	}

	void tWorldSpaceFloatingText::fMorphIntoFlyingText()
	{
		mFlyToScreenTimer = -1.f;
	}

	void tWorldSpaceFloatingText::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tWorldSpaceFloatingText, tWorldSpaceText, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			;

		vm.fRootTable( ).Bind( _SC("WorldSpaceFloatingText"), classDesc );
	}
} }
