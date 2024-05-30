#include "ToolsPch.hpp"
#include "iAssetGenPlugin.hpp"
#include "tAssetPluginDll.hpp"
#include "tLoadInPlaceFileBase.hpp"

namespace Sig
{
    namespace
    {
        class tAddDependency
        {
        public:
            tLoadInPlaceFileBase& mOutputFileObject;
            tFilePathPtr mDependencyPath;
            mutable tLoadInPlaceResourcePtr* mResult;

            tAddDependency( tLoadInPlaceFileBase& outputFileObject, const tFilePathPtr& dependencyPath )
                : mOutputFileObject( outputFileObject )
                , mDependencyPath( dependencyPath )
                , mResult( NULL )
            { }

            b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
            {
                iAssetGenPlugin* plug = assetPlugin.fGetAssetGenPluginInterface( );
                if( plug )
                {
                    mResult = plug->fAddDependencyInternal( mOutputFileObject, mDependencyPath );
                    if( mResult )
                        return false; // stop searching

                }
                return true;
            }

        };
    }

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
        tAddDependency adder( outputFileObject, dependencyPath );
        tAssetPluginDllDepot::fInstance( ).fForEachPlugin( adder );
		
		if( !adder.mResult )
			log_warning( "Unrecognized file type specified as resource dependency: [" << dependencyPath << "], ignoring." );

        return adder.mResult;
	}

}
