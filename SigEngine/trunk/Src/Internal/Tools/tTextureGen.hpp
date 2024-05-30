#ifndef __tTextureGen__
#define __tTextureGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{

	///
	/// \brief Asset plugin acces point for converting texture files.
	class tools_export tTextureGen
		: public iAssetPlugin
		, public iAssetGenPlugin
	{
	public:

		static tFilePathPtr fCreateResourceNameFromInputPath( const tFilePathPtr& inputPath );

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

#endif//__tTextureGen__

