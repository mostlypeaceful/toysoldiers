#ifndef __Tatml__
#define __Tatml__
#include "Gfx/tTextureFile.hpp"

namespace Sig { class tTextureSysRam; class tTextureAtlasSysRam; }

namespace Sig { namespace Tatml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsTatmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fTatmlPathToTatb( const tFilePathPtr& path );

	class tools_export tFile
	{
	public:
		u32								mVersion;
		u32								mSubWidth;
		u32								mSubHeight;
		Gfx::tTextureFile::tSemantic	mSemantic;
		Gfx::tTextureFile::tFormat		mFormat;
		tFilePathPtrList				mTexturePaths;
	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		void fDetermineNumTexturesXandY( u32& numTexturesX, u32& numTexturesY ) const;
		void fToTextureArray( tTextureSysRam& atlas );
		void fToTextureAtlas( tTextureAtlasSysRam& atlas );
	};

}}

#endif//__Tatml__
