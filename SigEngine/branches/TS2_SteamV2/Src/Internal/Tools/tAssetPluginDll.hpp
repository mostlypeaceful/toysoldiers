#ifndef __tAssetPluginDll__
#define __tAssetPluginDll__
#include "iAssetPlugin.hpp"
#include "Win32Include.hpp"
#include "tStrongPtr.hpp"


namespace Sig
{
	class tools_export tAssetPluginDll;

	///
	/// \brief Smart pointer type used to manage lifetime of tAssetPluginDll instances.
	typedef tStrongPtr<tAssetPluginDll> tAssetPluginDllPtr;

	///
	/// \brief Provides access to exported functionality from an AssetPlugin dll,
	/// specifically to the derived iAssetPlugin objects contained within.
	class tools_export tAssetPluginDll : public tUncopyable
	{
	public:

		static const char*						fGetPluginExt( );
		typedef tGrowableArray<iAssetPlugin*>	tPluginList;

		///
		/// This function should return iAssetGen::cVersion. The expected name of this
		/// function is fGetPluginVersion.
		typedef u32							(*tGetPluginVersion)	(void);

		///
		/// \brief This function should allocate plugin objects and push them back onto the supplied list.
		/// These exact same plugins (in the exact same order) will be returned in fDestroyPlugins. The expected
		/// name of this function is fCreatePlugins.
		typedef void						(*tCreatePlugins)		(tPluginList&);

		///
		/// \brief If plugin objects were allocated on the heap in fCreatePlugins, 
		/// then they should be freed in this function. The expected name of this function
		/// is fDestroyPlugins.
		typedef void						(*tDestroyPlugins)		(const tPluginList&);

	private:

		HMODULE								mDllHandle;
		tGetPluginVersion					mGetPluginVersion;
		tCreatePlugins						mCreatePlugins;
		tDestroyPlugins						mDestroyPlugins;
		tPluginList							mPluginObjects;

	public:

		tAssetPluginDll( HMODULE dllHandle );
		~tAssetPluginDll( );

		b32									fValid( ) const;
		inline const tPluginList&			fGetPluginObjects( ) const { return mPluginObjects; }

	private:

		void								fFreeDll( );
		void								fExtractDll( HMODULE dllHandle );
	};

	///
	/// \brief Manages, loads, and stores all plugin dlls relevant
	/// to this session (as dictated by the current project file).
	class tools_export tAssetPluginDllDepot : public tGrowableArray<tAssetPluginDllPtr>
	{
		declare_singleton_define_own_ctor_dtor( tAssetPluginDllDepot );

	public:

		///
		/// \brief Finds and opens the current project file, and loads
		/// the specified plugins.
		void fLoadPluginsBasedOnCurrentProjectFile( );

		///
		/// \brief Iterates over each plugin object, calling the templatized
		/// callback functor object at each valid one.
		/// \note The signature of tForEach is:
		/// b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin );
		/// it should return true to continue, or false to terminate iteration.
		template<class tForEach>
		void fForEachPlugin( const tForEach& forEach )
		{
			for( u32 i = 0; i < fCount( ); ++i )
			{
				const tAssetPluginDll::tPluginList& plugins = (*this)[i]->fGetPluginObjects( );

				for( u32 j = 0; j < plugins.fCount( ); ++j )
				{
					iAssetPlugin* ap = plugins[j];
					if( !ap )
						continue;

					if( !forEach( (*this)[i], *ap ) )
						return;
				}
			}
		}

	private:

		tAssetPluginDllDepot( );
		~tAssetPluginDllDepot( );

		void fCullPluginsByConfiguration( tFilePathPtrList& pluginFileNames );
		void fLoadPlugins( const tFilePathPtrList& pluginFileNames );
	};

}

#endif//__tAssetPluginDll__

