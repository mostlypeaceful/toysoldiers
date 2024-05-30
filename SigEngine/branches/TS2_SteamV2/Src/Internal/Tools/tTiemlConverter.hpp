#ifndef __tTiemlConverter__
#define __tTiemlConverter__
#include "Tieml.hpp"
#include "tFileWriter.hpp"
#include "Editor/tEditableTileDb.hpp"

namespace Sig
{
	namespace Gfx { class tMaterial; }

	class tMesh;
	class tTextureSysRam;

	///
	/// \brief Converts Tieml files to the binary/game equivalent, Sigb files.
	/// Used in AssetGen tool, but could conceivably be used in other contexts.
	class tools_export tTiemlConverter : public tSceneGraphFile
	{

		Tieml::tFile		mTiemlFile;
		tFilePathPtr		mResourcePath;
		tEditableTileDb		mDatabase;

	public:

		tTiemlConverter( );
		~tTiemlConverter( );

		///
		/// \brief Load the Tieml file.
		/// \return False if the file fails to load.
		b32 fLoadTiemlFile( const tFilePathPtr& TiemlFilePath, const tFilePathPtr& outputResourcePath );

		///
		/// \brief Convert platform-independent characteristics of the specified Tieml file. This should
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
		/// \brief Gets a constructed tile model path from the database. From a randomized tile or specified.
		tFilePathPtr fConstructModelFilePath( const Tieml::tTile& tile );

		/// 
		/// \brief Retrieves the script path for the associated script node.
		tFilePathPtr fGetScriptPath( u32 scriptGuid );

		///
		/// \brief Get Tieml file reference being converted.
		const Tieml::tFile& fTiemlFile( ) const { return mTiemlFile; }

		///
		/// \brief Get the resource output path.
		const tFilePathPtr& fGetResourcePath( ) const { return mResourcePath; }

	private:
	};
}

#endif//__tTiemlConverter__
