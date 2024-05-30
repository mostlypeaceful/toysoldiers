#include "MayaPluginPch.hpp"
#include "tMayaTexturePathFixerButton.hpp"


namespace Sig
{
	tMayaTexturePathFixerButton::tMayaTexturePathFixerButton( wxWindow* parent, const char* label )
		: tWxSlapOnButton( parent, label )
	{
	}

	void tMayaTexturePathFixerButton::fOnControlUpdated( )
	{
		//iterate through all textures
		for( MItDependencyNodes it( MFn::kFileTexture ); !it.isDone( ); it.next( ) )
		{
			// attach a dependency node to the file node
			MFnDependencyNode fn( it.item( ) );

			// get the attribute for the full texture path
			MPlug ftn = fn.findPlug("ftn");

			// get the filename from the attribute
			MString filename;
			ftn.getValue(filename);

			// attempt to convert to local machine

			tFilePathPtr path = ToolsPaths::fMakeResRelative( tFilePathPtr( filename.asChar( ) ), true );
			if( !path.fNull( ) )
			{
				// fix up this path
				path = ToolsPaths::fMakeResAbsolute( path );

				cout << path.fCStr() << endl;
				filename = path.fCStr( );
				ftn.setValue(filename);
			}
		}
	}

}

