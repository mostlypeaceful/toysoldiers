#ifndef __tMotionMapAssetPlugin__
#define __tMotionMapAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Momap { class tFile; }


	///
	/// \brief Converts .momap files
	class tools_export tMotionMapAssetGenPlugin : public iAssetGenPlugin
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
	/// \brief Asset plugin acces point for .momap-related plugins.
	class tools_export tMotionMapAssetPlugin : public iAssetPlugin
	{
		tMotionMapAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tMotionMapAssetPlugin__

