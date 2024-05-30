#ifndef __tSwfAssetPlugin__
#define __tSwfAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"
#include "tSwfFile.hpp"

namespace Sig
{
	class tools_export tSwfFileConverter : public tSwfFile
	{
	public:
		//return amount read/written. 0 = function failed in some way
		u32 fReadFile( const tFilePathPtr& input );
		u32 fWriteFile( const tFilePathPtr& output, const tPlatformId& pid ) const;
	};

	class tools_export tSwfAssetPlugin :
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

#endif//__tSwfAssetPlugin__
