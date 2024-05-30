#ifndef __tDataTableAssetGen__
#define __tDataTableAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Locml { class tFile; }

	///
	/// \brief Converts .Locml files exported from maya to .sklb files suitable for the game engine.
	class tools_export tDataTableAssetGenPlugin : public iAssetGenPlugin
	{
		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

	};

	///
	/// \brief Asset plugin acces point for .Locml-related plugins.
	class tools_export tDataTableAssetPlugin : public iAssetPlugin
	{
		tDataTableAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tDataTableAssetGen__

