#ifndef __tXmlAssetPlugin__
#define __tXmlAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"
#include "tXmlFile.hpp"

namespace Sig
{
	class tools_export tXmlFileConverter : public tXmlFile
	{
	public:
		//return amount read/written. 0 = function failed in some way
		u32 fReadFile( const tFilePathPtr& input );
		u32 fWriteFile( const tFilePathPtr& output, const tPlatformId& pid ) const;
	};

	class tools_export tXmlAssetPlugin :
		public iAssetPlugin,
		public iAssetGenPlugin
	{
	public:

		virtual iAssetGenPlugin* fGetAssetGenPluginInterface( )	{ return this; }

        virtual tLoadInPlaceResourcePtr* fAddDependencyInternal( 
            tLoadInPlaceFileBase& outputFileObject, 
            const tFilePathPtr& dependencyPath );

		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );
	};
}//Sig

#endif//__tXmlAssetPlugin__
