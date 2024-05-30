#ifndef __tAnimlConverter__
#define __tAnimlConverter__
#include "Animl.hpp"
#include "Anipk.hpp"
#include "Sklml.hpp"
#include "tAnimPackFile.hpp"

namespace Sig
{
	class tFileWriter;

	class tools_export tAnimlConverter : public tAnimPackFile
	{
	private:

		Anipk::tFile					mAnipkFile;
		Sklml::tFile					mSklmlFile;

		struct tExportAnim : public Animl::tFile
		{
			std::string mSimpleName;
		};

		tGrowableArray< tExportAnim >	mExportFiles;
		tFilePathPtr mResourcePath;

	public:

		tAnimlConverter( );
		~tAnimlConverter( );

		///
		/// \brief Load the sklml file.
		/// \return False if the file fails to load.
		b32 fLoadAnimlFiles( const tFilePathPtrList& animlFilePaths, const tFilePathPtr& outputResourcePath );

		///
		/// \brief Convert platform-independent characteristics of the specified animl file. This should
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

	private:

		b32 fConvertAnim( 
			tKeyFrameAnimation& oanim, 
			const tExportAnim& ianim );
		b32 fConvertBone( 
			tKeyFrameAnimation& oanim, 
			tKeyFrameAnimation::tBone& obone,
			const Animl::tBone& ibone,
			const Animl::tDeltaCompressionResult& compressionResult,
			b32 refFrame );
		void fConvertReferenceFrame( const Animl::tFile& animl, const Animl::tBone& animlBone, Animl::tKeyFrameList& newKeysOut );
		b32 fConvertBoneToParentSpace( const Animl::tFile& animl, const Animl::tBone& animlBone, Animl::tKeyFrameList& newKeysOut );
	};
}

#endif//__tAnimlConverter__

