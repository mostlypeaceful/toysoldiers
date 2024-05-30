#include "BasePch.hpp"
#include "tCameraControllerStack.hpp"

namespace Sig { namespace Gfx
{
	void tCameraControllerStack::fStep( f32 dt )
	{
		for( u32 i = 0; i < mStack.fCount( ); ++i )
			mStack[ i ]->fOnTick( dt );
		if( mStack.fCount( ) > 0 && mStack.fBack( )->fWantsAutoPop( ) )
			fPopCamera( );
	}
	void tCameraControllerStack::fClear( )
	{
		// deactivate previous "current" camera
		if( mStack.fCount( ) > 0 )
			mStack.fBack( )->fOnActivate( false );
		// clear stack
		for( s32 i = mStack.fCount( ) - 1; i >= 0; --i )
			mStack[ i ]->fOnRemove( );
		mStack.fSetCount( 0 );
	}
	void tCameraControllerStack::fPushCamera( const tCameraControllerPtr& camera, u32 index )
	{
		// deactivate previous "current" camera
		if( mStack.fCount( ) > 0 )
			mStack.fBack( )->fOnActivate( false );

		// push new camera to top of stack
		mStack.fInsertSafe( index, camera );

		// activate new camera
		mStack.fBack( )->fOnActivate( true );
	}
	void tCameraControllerStack::fPopCamera( )
	{
		if( mStack.fCount( ) > 0 )
		{
			mStack.fBack( )->fOnActivate( false );
			mStack.fBack( )->fOnRemove( );
			mStack.fPopBack( );
		}
		if( mStack.fCount( ) > 0 )
			mStack.fBack( )->fOnActivate( true );
	}
	void tCameraControllerStack::fEraseOrdered( u32 index ) 
	{ 
		if( mStack.fCount( ) > index && mStack[ index ]->fIsActive( ) ) 
			mStack[ index ]->fOnActivate( false );

		mStack[ index ]->fOnRemove( );
		mStack.fEraseOrdered( index ); 

		if( mStack.fCount( ) > 0 && !mStack.fBack( )->fIsActive( ) )
			mStack.fBack( )->fOnActivate( true );
	}
}}

