#ifndef __tXbDm__
#define __tXbDm__

namespace Sig
{

	///
	/// \brief Global singleton object responsible for managing connection
	/// with devkit, loading xbdm.dll, etc.
	class toolsxdk_export tXbDm
	{
		declare_singleton_define_own_ctor_dtor( tXbDm );
	private:
		HMODULE mDllHandle;
		tFilePathPtr mXdkBinPath;
		std::string mDevkitName;
	public:
		const tFilePathPtr& fXdkBinPath( ) const { return mXdkBinPath; }
		const std::string& fDevkitName( ) const { return mDevkitName; }
		b32 fIsConnectedToDevkit( ) const { return mDllHandle && mDevkitName.length( ) > 0; }
		b32 fLoadLibrary( );
		void fSetDevkitName( const std::string& name );
		b32 fEnsureDirExists( const tFilePathPtr& xbSubDirDestSimple ) const;
		b32 fCopyResourcesToDevkit( ) const;
		b32 fCopyFilesToDevkitFolder( const tFilePathPtrList& src, const tFilePathPtr& xbSubDirDest, b32 forceCopy = false ) const;
		b32 fLaunchXex( const tFilePathPtr& xexPath, const std::string& args = std::string( ), b32 copyOnly = false, b32 minimal = false ) const;
		b32 fLaunchWatson( ) const;
	private:
		tXbDm( );
		~tXbDm( );
	};

}

#endif//__tXbDm__

