#ifndef __tStandardAssetPlugin__
#define __tStandardAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	///
	/// \brief Asset plugin acces point standard, "built-in" asset gen features,
	/// such as invoking "sigcmd" batch files.
	class tools_export tStandardAssetPlugin : 
		public iAssetPlugin,
		public iAssetGenPlugin
	{
	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return this; }

		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );
	};
}

#endif//__tStandardAssetPlugin__
