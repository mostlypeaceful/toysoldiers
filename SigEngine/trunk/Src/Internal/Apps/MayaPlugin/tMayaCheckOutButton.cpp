#include "MayaPluginPch.hpp"
#include "tMayaCheckOutButton.hpp"


namespace Sig
{
	tMayaCheckOutButton::tMayaCheckOutButton( wxWindow* parent, const char* label )
		: tWxSlapOnButton( parent, label )
	{
	}

	void tMayaCheckOutButton::fOnControlUpdated( )
	{
		MString mayaCurrentFile = MFileIO::currentFile( );
		if( mayaCurrentFile.length( ) > 0 )
			ToolsPaths::fCheckout( tFilePathPtr( mayaCurrentFile.asChar( ) ) );
	}

}

