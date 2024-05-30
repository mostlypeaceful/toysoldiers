// Point Capture UI
#ifndef __tPointCaptureUI__
#define __tPointCaptureUI__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tPointCaptureUI : public tScriptedControl
	{
	public:
		explicit tPointCaptureUI( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tPointCaptureUI( ) { }

		void fShow( b32 show );
		u32 fCurrentOwner( );
		void fSetPercent( u32 team, f32 percent );
		void fCapture( u32 team );
		void fLost( u32 toTeam );
		void fDisputed( b32 isDisputed );

		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
	};

	typedef tRefCounterPtr< tPointCaptureUI > tPointCaptureUIPtr;
} }

#endif //__tPointCaptureUI__