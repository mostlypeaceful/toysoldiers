#ifndef __tScreenSpaceNotification__
#define __tScreenSpaceNotification__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig
{

namespace Gui
{
	class tScreenSpaceNotification : public tScriptedControl
	{
	public:
		explicit tScreenSpaceNotification( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tScreenSpaceNotification( );

		void fEnable( b32 enable );
		b32 fIsEnabled( ) const { return mEnabled; }
		void fSpawnText( const char* text, const Math::tVec4f& color );
		void fSpawnText( const tLocalizedString& text, const Math::tVec4f& color );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		b32 mEnabled;
		tUserPtr mUser;
	};

	typedef tRefCounterPtr< tScreenSpaceNotification > tScreenSpaceNotificationPtr;
}}

#endif //__tScreenSpaceNotification__