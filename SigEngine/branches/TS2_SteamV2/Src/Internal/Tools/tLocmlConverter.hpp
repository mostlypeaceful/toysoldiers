#ifndef __tLocmlConverter__
#define __tLocmlConverter__
#include "Locml.hpp"
#include "tLocalizationFile.hpp"

namespace Sig
{
	class tFileWriter;

	class tools_export tLocmlConverter : public tLocalizationFile
	{
		Locml::tFile mLocml;
	public:
		const Locml::tFile& fLocmlFile( ) const { return mLocml; }
		b32 fLoadLocmlFile( const tFilePathPtr& path );
		b32 fConvertPlatformCommon( );
		b32 fConvertPlatformSpecific( tPlatformId pid );
		void fOutput( tFileWriter& writer, tPlatformId pid );
	};

}

#endif//__tLocmlConverter__

