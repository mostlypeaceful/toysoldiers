#ifndef __tTiledbConverter__
#define __tTiledbConverter__
#include "Tiledb.hpp"
#include "tLocalizationFile.hpp"
#include "tTilePackage.hpp"

namespace Sig
{
	class tFileWriter;

	class tools_export tTiledmlConverter : public tTilePackage
	{
		Tiledml::tFile mTiledb;
	public:
		const Tiledml::tFile& fTiledbFile( ) const { return mTiledb; }
		b32 fLoadTiledbFile( const tFilePathPtr& path );
		b32 fConvertPlatformCommon( );
		b32 fConvertPlatformSpecific( tPlatformId pid );
		void fOutput( tFileWriter& writer, tPlatformId pid );
	};

}

#endif//__tTiledbConverter__

