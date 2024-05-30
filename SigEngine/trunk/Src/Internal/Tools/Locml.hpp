#ifndef __Locml__
#define __Locml__

namespace Sig { namespace Locml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsLocmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fLocmlPathToLocb( const tFilePathPtr& path );


	class tools_export tStringEntry
	{
	public:
		std::string		mName;
		std::wstring	mText;
	};
	typedef tGrowableArray< tStringEntry > tStringEntryArray;

	class tools_export tPathEntry
	{
	public:
		std::string		mName;
		tFilePathPtr	mPath;
	};
	typedef tGrowableArray< tPathEntry > tPathEntryArray;

	class tools_export tFile
	{
	public:
		u32					mVersion;
		tStringEntryArray	mStringTable;
		tPathEntryArray		mPathTable;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
	};

}}

#endif//__Locml__
