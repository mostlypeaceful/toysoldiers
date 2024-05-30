//------------------------------------------------------------------------------
// \file iSigEdPlugin.hpp - 12 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __iSigEdPlugin__
#define __iSigEdPlugin__
#include "Editor/tEditableSgFileRefEntity.hpp"

namespace Sig
{
	class tools_export tAssetPluginDll;
	typedef tStrongPtr<tAssetPluginDll> tAssetPluginDllPtr;
	class iAssetPlugin;

	struct tSigEdPluginInputOutput
	{
		tSgFileRefEntityPtr& mInputEntity;
		tGrowableArray<f32>& mOutputRadii;

		tSigEdPluginInputOutput( tSgFileRefEntityPtr& entity, tGrowableArray<f32>& output );

		b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const;
	};

	class iSigEdPlugin
	{
	public:
		virtual void fProcessInputOutput( const tSigEdPluginInputOutput& inputOutput ) = 0;
	};
}


#endif//__iSigEdPlugin__
