#ifndef __tMayaEvent__
#define __tMayaEvent__
#include "tDelegate.hpp"
#include "tStrongPtr.hpp"

namespace Sig
{
	///
	/// \brief Application-wide maya event notifier.
	class tMayaEvent
	{
	public:

		static const char* fEventNameIdle( )		{ return "idle"; }
		static const char* fEventNameQuit( )		{ return "quitApplication"; }
		static const char* fEventNameSelChanged( )	{ return "SelectionChanged"; }
		static const char* fEventNameSceneOpened( )	{ return "SceneOpened"; }
		static const char* fEventNameSceneImported( ){ return "SceneImported"; }

		static void fLogEventNames( );

	public:

		typedef tDelegate<void ( )> tCallback;

	private:

		tCallback			mCallback;
		MCallbackId			mMayaCallbackId;

	public:

		///
		/// \brief	Creates the event callback
		tMayaEvent( const char* eventName, const tCallback& callback )
			: mCallback( callback )
		{
			// register the callback function with maya
			mMayaCallbackId = MEventMessage::addEventCallback( eventName, fOnMayaEvent, this );
		}

		///
		/// \brief Unregisters the callback functions
		~tMayaEvent( )
		{
			// make sure we delete the callback before the plugin quits !!!
			MEventMessage::removeCallback( mMayaCallbackId );
		}

	private:

		/// \brief	This function calls.
		static void fOnMayaEvent(void* data)
		{
			tMayaEvent* This = ( tMayaEvent* )data;
			This->mCallback( );
		}
	};

	typedef tStrongPtr<tMayaEvent> tMayaEventPtr;

}

#endif//__tMayaEvent__
