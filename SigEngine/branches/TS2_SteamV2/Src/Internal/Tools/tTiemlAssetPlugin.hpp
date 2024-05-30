#ifndef __tTiemlAssetGen__
#define __tTiemlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Tieml { class tFile; }

	class tools_export tTiemlAssetGenPlugin : public iAssetGenPlugin
	{
		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Tieml::tFile& TiemlFile );
	};

	class tools_export tTiemlAssetPlugin : public iAssetPlugin
	{
		tTiemlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tTiemlAssetGen__

