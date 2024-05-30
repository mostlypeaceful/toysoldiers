#ifndef __tDermlAssetGen__
#define __tDermlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Derml { class tFile; }


	///
	/// \brief Converts .derml files exported from maya to .mtlb files suitable for the game engine.
	class tools_export tDermlAssetGenPlugin : public iAssetGenPlugin
	{
		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Derml::tFile& dermlFile );
	};

	///
	/// \brief Asset plugin acces point for .derml-related plugins.
	class tools_export tDermlAssetPlugin : public iAssetPlugin
	{
		tDermlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tDermlAssetGen__

