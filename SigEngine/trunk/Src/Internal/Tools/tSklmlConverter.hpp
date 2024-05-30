//------------------------------------------------------------------------------
// \file tSklmlConverter.hpp - 11 Jul 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tSklmlConverter__
#define __tSklmlConverter__
#include "Sklml.hpp"
#include "tSkeletonFile.hpp"

namespace Sig
{
	class tFileWriter;

	///
	/// \class tSklmlConverter
	/// \brief Converts sklml to tSkeletonFile
	class tools_export tSklmlConverter : public tSkeletonFile
	{
	private:

		Sklml::tFile mExportFile;
		tFilePathPtr mResourcePath;

	public:

		tSklmlConverter( );
		~tSklmlConverter( );

		///
		/// \brief Load the sklml file.
		/// \return False if the file fails to load.
		b32 fLoadSklmlFile( const tFilePathPtr& sklmlFilePath, const tFilePathPtr& outputResourcePath );

		///
		/// \brief Convert platform-independent characteristics of the specified sklml file. This should
		/// be called prior to calling fConvertPlatformSpecific or fOutput.
		b32 fConvertPlatformCommon( );

		///
		/// \brief Convert platform-specific aspects of the specified file, for the specified platform.
		/// This method should be called after fConvertPlatformCommon and before fOutput.
		b32 fConvertPlatformSpecific( tPlatformId pid );

		///
		/// \brief Output the converted file for the specified platform. Should be called only
		/// after calling both fConvertPlatformCommon and fConvertPlatformSpecific.
		b32 fOutput( tFileWriter& ofile, tPlatformId pid );

		///
		/// \brief Get sigml file reference being converted.
		const Sklml::tFile& fSklmlFile( ) const { return mExportFile; }

	private:

		void fConvertBone( tBone& obone, const Sklml::tBone& ibone, const tBone * parent );
		void fCopyBone( tBone & obone, const Sklml::tBone & ibone, const tBone * parent ); 
		void fBuildSkeletonMaps( );
		u32 fLoadSkeletonMapFiles( 
			const Sklml::tFile & srcFile,
			tHashTable< tHashTablePtrInt, Sklml::tFile * > & skels );

		const Sklml::tBone* fFindBone( 
			const std::string & name, 
			const Sklml::tBone & root, 
			const Sklml::tBone *& parent );
		void fBuildSkeletonMap( 
			tSkeletonMap & map, 
			const Sklml::tSkeleton & source, 
			const Sklml::tBone & tgtBone, 
			const Sklml::tBone * tgtParent );

	};
}

#endif//__tSklmlConverter__

