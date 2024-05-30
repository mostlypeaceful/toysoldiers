#include "ToolsPch.hpp"
#include "iAssetPlugin.hpp"
#include "tLoadInPlaceFileBase.hpp"

#include "Sigml.hpp"
#include "tSceneGraphFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "tTextureSysRam.hpp"
#include "Anipk.hpp"
#include "tAnimPackFile.hpp"
#include "Fxml.hpp"
#include "FX/tFxFile.hpp"
#include "Gui/tFont.hpp"
#include "tDataTableFile.hpp"

namespace Sig
{

	tFilePathPtr iAssetGenPlugin::fCreateRelativeOutputPath( const tFilePathPtr& inputFile, const char* outputExtension )
	{
		return tFilePathPtr::fSwapExtension( ToolsPaths::fMakeResRelative( inputFile ), outputExtension );
	}

	tFilePathPtr iAssetGenPlugin::fCreateAbsoluteOutputPath( tPlatformId outputPlatform, const tFilePathPtr& relativeOutputPath )
	{
		return ToolsPaths::fMakeGameAbsolute( relativeOutputPath, outputPlatform );
	}

	tLoadInPlaceResourcePtr* iAssetGenPlugin::fAddDependency( tLoadInPlaceFileBase& outputFileObject, const tFilePathPtr& dependencyPath )
	{
		tLoadInPlaceResourcePtr* o = 0;

		if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".nut" ) )
		{
			const tFilePathPtr filePath( std::string( dependencyPath.fCStr( ) ) + "b" );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tScriptFile>( filePath ) );
		}
		else if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".fntml" ) )
		{
			const tFilePathPtr filePath = tFilePathPtr::fSwapExtension( dependencyPath, ".fntb" );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gui::tFont>( filePath ) );
		}
		else if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".goaml" ) )
		{
			const tFilePathPtr filePath = tFilePathPtr::fSwapExtension( dependencyPath, ".nutb" );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tScriptFile>( filePath ) );
		}
		else if( StringUtil::fCheckExtension( dependencyPath.fCStr( ), ".tab" ) )
		{
			const tFilePathPtr filePath( std::string( dependencyPath.fCStr( ) ) + "b" );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tDataTableFile>( filePath ) );
		}
		else if( Sigml::fIsSigmlFile( dependencyPath ) )
		{
			const tFilePathPtr filePath = Sigml::fSigmlPathToSigb( dependencyPath );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tSceneGraphFile>( filePath ) );
		}
		else if( Fxml::fIsFxmlFile( dependencyPath ) )
		{
			const tFilePathPtr filePath = Fxml::fFxmlPathToFxb( dependencyPath );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake< FX::tFxFile >( filePath ) );
		}
		else if( Anipk::fIsAnipkFile( dependencyPath ) )
		{
			const tFilePathPtr filePath = Anipk::fAnipkPathToAnib( dependencyPath );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tAnimPackFile>( filePath ) );
		}
		else if( tTextureSysRam::fRecognizedExtension( dependencyPath.fCStr( ) ) )
		{
			const tFilePathPtr filePath = tTextureSysRam::fCreateBinaryPath( dependencyPath );
			o = outputFileObject.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( filePath ) );
		}
		else
		{
			log_warning( 0, "Unrecognized file type specified as resource dependency: [" << dependencyPath << "], ignoring." );
		}

		return o;
	}

}
