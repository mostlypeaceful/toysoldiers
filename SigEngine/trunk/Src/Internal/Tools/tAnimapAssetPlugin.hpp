#ifndef __tAnimapAssetPlugin__
#define __tAnimapAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Animap { class tFile; }


	///
	/// \brief Converts .animap files
	class tools_export tAnimapAssetGenPlugin : public iAssetGenPlugin
	{
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

	///
	/// \brief Asset plugin acces point for .animap-related plugins.
	class tools_export tAnimapAssetPlugin : public iAssetPlugin
	{
		tAnimapAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tAnimapAssetPlugin__

