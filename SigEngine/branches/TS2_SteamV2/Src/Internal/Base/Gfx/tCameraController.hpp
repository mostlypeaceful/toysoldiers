#ifndef __tCameraController__
#define __tCameraController__
#include "tUser.hpp"
#include "tDelegate.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief The look at callback provides a generic interface between
	/// camera controllers and what they're looking at; basically it's just an
	/// arbitrary global or member function that will return a look at point.
	/// It is combined with shared pointer semantics so they can be safely
	/// shared between objects and camers.
	class tLookAtCallback : 
		public tDelegate<void ( Math::tMat3f& lookAtReferenceFrame )>,
		public tRefCounter
	{
	};

	typedef tRefCounterPtr<tLookAtCallback> tLookAtCallbackPtr;

	class tCamera;

	class tCameraShake;
	typedef tRefCounterPtr<tCameraShake> tCameraShakePtr;
	
	class tCameraHitByExplosion;
	typedef tRefCounterPtr<tCameraHitByExplosion> tCameraHitByExplosionPtr;

	
	///
	/// \brief Base class providing common base functionality and
	/// defining interface for derived camera controller types. Camera 
	/// controllers are maintained in a stack on tViewport, and are the
	/// primary manipulators of the viewport's actual camera object.
	/// \see tViewport
	class base_export tCameraController : public tUncopyable, public tRefCounter
	{
		define_dynamic_cast_base( tCameraController );
	protected:
		tViewportPtr mViewport;
		tGrowableArray< tUserPtr >	mUsers;
		f32							mCameraShakeScale;
	private:
		tCameraShakePtr				mCameraShake;
		tCameraHitByExplosionPtr	mCameraExplosion;
		tLookAtCallbackPtr			mLookAtCb;
		b32							mIsActive;

	public:

		tCameraController( const tViewportPtr& viewport );
		virtual ~tCameraController( );

		void fAddUser( const tUserPtr& user );
		void fRemoveUser( const tUserPtr& user );
		void fClearUsers( );
		void fBeginCameraShake( const Math::tVec2f& strength, f32 length );
		virtual void fHitCameraWithExplosion( const Math::tVec3f& explosionPos, f32 explosionStrength, f32 timeToSnap );

		const tViewportPtr& fViewport( ) const { return mViewport; }

		void fSetLookAtCallback( const tLookAtCallbackPtr& lookAtCb ) { mLookAtCb = lookAtCb; }
		const tLookAtCallbackPtr& fGetLookAtCallback( ) const { return mLookAtCb; }
		
		virtual b32 fWantsAutoPop( ) const { return false; } // for cameras that are at the top of the camera controller stack, if this returns true, it will get popped after stepping all cameras
		virtual void fOnTick( f32 dt ) = 0;
		virtual void fOnActivate( b32 active ) { mIsActive = active; }
		virtual void fOnRemove( ) { }
		inline b32 fIsActive( ) const { return mIsActive; }

		void fSetCameraShakeScale( f32 scale ) { mCameraShakeScale = scale; }

	protected:

		b32 fEvaluateLookAt( Math::tMat3f& lookAtReferenceFrame );
		void fStepCameraShake( f32 dt, tCamera& camera );
		void fStepCameraExplosion( f32, tCamera& camera );
	};

	typedef tRefCounterPtr< tCameraController > tCameraControllerPtr;

}}


#endif//__tCameraController__

