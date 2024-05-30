//------------------------------------------------------------------------------
// \file tWorldToScreenSpaceText.cpp - 02 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tWorldToScreenSpaceText.hpp"

namespace Sig { namespace Gui
{

	//------------------------------------------------------------------------------
	// tWorldToScreenSpaceText
	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fCreate( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas )
	{
		mScreenSpaceText.fNewArray( 1 );
		mScreenSpaceText[ 0 ].mUser = tUserPtr( &user );
		mScreenSpaceText[ 0 ].mText = fCreateText( font, user );
		parentCanvas.fAddChild( tCanvasPtr( mScreenSpaceText[ 0 ].mText.fGetRawPtr( ) ) );
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fCreate( const tResourcePtr& font, const tUserArray& userArray, tCanvasFrame& parentCanvas )
	{
		mScreenSpaceText.fNewArray( userArray.fCount( ) );
		for( u32 i = 0; i < mScreenSpaceText.fCount( ); ++i )
		{
			mScreenSpaceText[ i ].mUser = userArray[ i ];
			mScreenSpaceText[ i ].mText = fCreateText( font, *userArray[ i ] );
			parentCanvas.fAddChild( tCanvasPtr( mScreenSpaceText[ i ].mText.fGetRawPtr( ) ) );
		}
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fAdd( const tResourcePtr& font, tUser& user, tCanvasFrame& parentCanvas )
	{
		mScreenSpaceText.fPushBack( tEntry( ) );
		mScreenSpaceText.fBack( ).mUser = tUserPtr( &user );
		mScreenSpaceText.fBack( ).mText = fCreateText( font, user );
		parentCanvas.fAddChild( tCanvasPtr( mScreenSpaceText.fBack( ).mText.fGetRawPtr( ) ) );
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fOnSpawn( )
	{
		fRunListInsert( cRunListPreRenderST );
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fOnDelete( )
	{
		fClear( );
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fClear( )
	{
		for( u32 i = 0; i < mScreenSpaceText.fCount( ); ++i )
			mScreenSpaceText[ i ].mText->fRemoveFromParent( );
		mScreenSpaceText.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fPreRenderST( f32 dt )
	{
		for( u32 i = 0; i < mScreenSpaceText.fCount( ); ++i )
		{
			const Math::tVec3f screenPos = mScreenSpaceText[ i ].mUser->fViewport( )->fProjectToScreen( fObjectToWorld( ).fGetTranslation( ) );
			if( fInBounds( screenPos.z, 0.f, 1.f ) && !mScreenSpaceText[ i ].mUser->fIsViewportVirtual( ) )
			{
				mScreenSpaceText[ i ].mText->fSetInvisible( false );
				mScreenSpaceText[ i ].mText->fSetPosition( screenPos.x, screenPos.y, 0.f );
			}
			else
				mScreenSpaceText[ i ].mText->fSetInvisible( true );
		}
	}

	//------------------------------------------------------------------------------
	tTextPtr tWorldToScreenSpaceText::fCreateText( const tResourcePtr& font, const tUser& user )
	{
		const b32 dropShadow = true; // TODO parameterize

		tTextPtr text( NEW tText( dropShadow ) );

		text->fSetFont( font );
		text->fSetInvisible( true );

		const Math::tRect safeRect = user.fComputeViewportRect( );
		text->fSetScissorRect( safeRect );

		return text;
	}

	//------------------------------------------------------------------------------
	void tWorldToScreenSpaceText::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tWorldToScreenSpaceText, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			;

		vm.fRootTable( ).Bind( _SC("WorldToScreenSpaceText"), classDesc );
	}

}}

