#ifndef __iAssetGen__
#define __iAssetGen__

namespace Sig
{
	class tLoadInPlaceFileBase;
	class tLoadInPlaceResourcePtr;

	class iAssetGenPlugin;
	class iMaterialGenPlugin;
	class iSigEdPlugin;


	///
	/// \brief Interface to one or more of the concrete asset plugin type interfaces.
	/// This type is simply a wrapper providing a unified access point to the various plugin types,
	/// facilitating the AssetPluginDll mechanism.
	/// \see AssetPluginDll.
	class tools_export iAssetPlugin
	{
	public:

		virtual ~iAssetPlugin( ) { }
		virtual iAssetGenPlugin*		fGetAssetGenPluginInterface( )		{ return 0; }
		virtual iMaterialGenPlugin*		fGetMaterialGenPluginInterface( )	{ return 0; }
		virtual iSigEdPlugin*			fGetSigEdPluginInterface( )			{ return 0; }
	};

	/// Increment this by one if you alter the interface in any way, or the interface of any of the exposed interfaces.
#	define asset_plugin_version 4u

}


#endif//__iAssetGen__
