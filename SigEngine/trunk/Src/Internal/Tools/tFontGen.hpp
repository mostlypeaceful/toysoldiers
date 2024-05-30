#ifndef __tFontGen__
#define __tFontGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{

	///
	/// \brief Asset plugin acces point for converting font files.
	class tools_export tFontGen 
		: public iAssetPlugin
		, public iAssetGenPlugin
	{
	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return this; }

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
}


#endif//__tFontGen__
