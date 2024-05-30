#ifndef __tGoamlAssetGen__
#define __tGoamlAssetGen__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig
{
	namespace Goaml { class tFile; }


	///
	/// \brief Converts .goaml files
	class tools_export tGoamlAssetGenPlugin : public iAssetGenPlugin
	{
		virtual void fDetermineInputOutputs( 
			tInputOutputList& inputOutputsOut,
			const tFilePathPtrList& immediateInputFiles );

		virtual void fProcessInputOutputs(
			const tInputOutput& inputOutput,
			tFilePathPtrList& indirectGenFilesAddTo );

		void fAddDependencies( tFilePathPtrList& indirectGenFilesAddTo, const Goaml::tFile& goamlFile );
	};

	///
	/// \brief Asset plugin acces point for .derml-related plugins.
	class tools_export tGoamlAssetPlugin : public iAssetPlugin
	{
		tGoamlAssetGenPlugin		mAssetGenPlugin;

	public:

		virtual iAssetGenPlugin*	fGetAssetGenPluginInterface( )	{ return &mAssetGenPlugin; }
	};

}

#endif//__tGoamlAssetGen__

