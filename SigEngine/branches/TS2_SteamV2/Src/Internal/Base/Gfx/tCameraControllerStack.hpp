#ifndef __tCameraControllerStack__
#define __tCameraControllerStack__
#include "tCameraController.hpp"

namespace Sig { namespace Gfx
{
	class tCameraControllerStack
	{
	public:
		u32 fCount( ) const { return mStack.fCount( ); }
		b32 fIsEmpty( ) const { return mStack.fCount( ) == 0; }
		const tCameraControllerPtr& fTop( ) const { return mStack.fBack( ); }
		tCameraController& operator[]( u32 i ) { return *mStack[ i ]; }
		const tCameraController& operator[]( u32 i) const { return *mStack[ i ]; }
		void fStep( f32 dt );
		void fClear( );
		void fPushCamera( const tCameraControllerPtr& camera, u32 index = ~0 );
		void fPopCamera( );
		void fEraseOrdered( u32 index );

		template<class t>
		b32 fPopCamerasOfType( )
		{
			b32 found = false;

			for( u32 i = 0; i < mStack.fCount( ); ++i )
			{
				if( mStack[ i ]->fDynamicCast<t>( ) )
				{
					// calls this on this class so that the camera activation can be preserved
					fEraseOrdered( i );
					--i;
					found = true;
				}
			}

			return found;
		}

		template<class t>
		t* fFindCameraOfType( )
		{
			for( u32 i = 0; i < mStack.fCount( ); ++i )
			{
				t* p = mStack[ i ]->fDynamicCast<t>( );
				if( p ) 
					return p;
			}

			return NULL;
		}

		template<class t>
		u32 fIndexOfType( )
		{
			for( s32 i = mStack.fCount( ) - 1; i >= 0 ; --i )
			{
				t* p = mStack[ i ]->fDynamicCast<t>( );
				if( p ) 
					return i;
			}

			return ~0;
		}

	private:
		tGrowableArray<tCameraControllerPtr> mStack;
	};

}}

#endif//__tCameraControllerStack__

