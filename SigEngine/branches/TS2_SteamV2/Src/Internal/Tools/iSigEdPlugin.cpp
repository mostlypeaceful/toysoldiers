//------------------------------------------------------------------------------
// \file iSigEdPlugin.cpp - 12 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "iSigEdPlugin.hpp"
#include "tAssetPluginDll.hpp"

namespace Sig
{
	tSigEdPluginInputOutput::tSigEdPluginInputOutput( tSgFileRefEntityPtr& entity, tGrowableArray<f32>& output )
		: mInputEntity( entity )
		, mOutputRadii( output )
	{
	}

	b32 tSigEdPluginInputOutput::operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
	{
		iSigEdPlugin* sep = assetPlugin.fGetSigEdPluginInterface( );
		if( !sep )
			return true;

		sep->fProcessInputOutput( *this );

		return false;
	}
}
