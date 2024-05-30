//------------------------------------------------------------------------------
// \file tTtfAssetPlugin.hpp - 22 Jan 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTtfAssetPlugin__
#define __tTtfAssetPlugin__
#include "iAssetPlugin.hpp"
#include "iAssetGenPlugin.hpp"
#include "tTtfFile.hpp"

namespace Sig
{
	///
	/// \class tTtfFileConverter
	/// \brief 
	class tools_export tTtfFileConverter : public tTtfFile
	{
	public:
		//return amount read/written. 0 = function failed in some way
		u32 fReadFile( const tFilePathPtr& input );
		u32 fWriteFile( const tFilePathPtr& output, const tPlatformId& pid ) const;
	};

	///
	/// \class tTtfAssetPlugin
	/// \brief 
	class tools_export tTtfAssetPlugin : public iAssetPlugin, public iAssetGenPlugin
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
}

#endif//__tTtfAssetPlugin__
