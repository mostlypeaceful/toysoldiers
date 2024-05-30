#ifndef __tInUseIndicator__
#define __tInUseIndicator__

#include "Gui/tWorldSpaceScriptedControl.hpp"
#include "tUser.hpp"
namespace Sig { 
	
	class tPlayer;

	namespace Gui
{

	class tInUseIndicator : public tWorldSpaceScriptedControl
	{
	public:
		tInUseIndicator( const tUserArray& users );
		virtual ~tInUseIndicator( );
		void fShow( );
		void fShow( const tUserPtr& exceptThisGuy );
		void fHide( );
		void fSetIndicator( tPlayer* userInControl, u32 country );
		b32 fReadyForDeletion( ) { return mShouldDelete; }
	private:
		b32 mShouldDelete;
	};

	typedef tRefCounterPtr< tInUseIndicator > tInUseIndicatorPtr;

} } // end namespaces Sig::Gui

#endif //__tInUseIndicator__