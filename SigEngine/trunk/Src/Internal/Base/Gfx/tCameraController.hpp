#ifndef __tCameraController__
#define __tCameraController__
#include "tUser.hpp"

namespace Sig { namespace Gfx
{

	class tCamera;

	class tCameraShake;
	typedef tRefCounterPtr<tCameraShake> tCameraShakePtr;

	
	///
	/// \brief Base class providing common base functionality and
	/// defining interface for derived camera controller types. Camera 
	/// controllers are maintained in a stack on tViewport, and are the
	/// primary manipulators of the viewport's actual camera object.
	/// \see tViewport
	class base_export tCameraController : public tRefCounter
	{
		define_dynamic_cast_base( tCameraController );
		declare_uncopyable( tCameraController );

	protected:
		tViewportPtr mViewport;
		tGrowableArray< tUserPtr >	mUsers;
		f32							mCameraShakeScale;

	private:
		tGrowableArray<tCameraShakePtr> mCameraShakes;
		b32							mIsActive;

	public:

		tCameraController( const tViewportPtr& viewport );
		virtual ~tCameraController( );

		void fAddUser( const tUserPtr& user );
		void fRemoveUser( const tUserPtr& user );
		void fClearUsers( );
		void fBeginCameraShake( const Math::tVec2f& strength, f32 length );

		const tViewportPtr& fViewport( ) const { return mViewport; }
		
		virtual b32 fWantsAutoPop( ) const { return false; } // for cameras that are at the top of the camera controller stack, if this returns true, it will get popped after stepping all cameras
		virtual void fOnTick( f32 dt ) = 0;
		virtual void fOnActivate( b32 active ) { mIsActive = active; }
		virtual void fOnRemove( ) { }
		virtual void fOnWarp( ) { }
		virtual void fDebugDumpCameraStackFormat( std::stringstream& ss ) const { ss << fDebugTypeName( ); }

		inline b32 fIsActive( ) const { return mIsActive; }

	protected:
		void fStepCameraShake( f32 dt, tCamera& camera );
	};

	typedef tRefCounterPtr< tCameraController > tCameraControllerPtr;

}}


#endif//__tCameraController__

