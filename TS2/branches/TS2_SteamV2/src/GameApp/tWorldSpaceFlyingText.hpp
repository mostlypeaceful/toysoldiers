// World Space Flying Text
#ifndef __tWorldSpaceFlyingText__
#define __tWorldSpaceFlyingText__

#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tWorldSpaceFlyingText : public tScriptedControl
	{
	public:
		explicit tWorldSpaceFlyingText( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tWorldSpaceFlyingText( ) { }

		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
		void fSetTarget( const Math::tVec3f& target );
		void fSetText( const Sqrat::Object& textObj );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
	};
} }

#endif //__tWorldSpaceFlyingText__