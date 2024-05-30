#ifndef __tFxmlConverter__
#define __tFxmlConverter__
#include "Fxml.hpp"
#include "tSceneGraphFile.hpp"
#include "FX/tFxFile.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Gfx/tIndexBufferSysRam.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	namespace Gfx { class tMaterial; }

	///
	/// \brief Converts Fxml files to the binary/game equivalent, Sigb files.
	/// Used in AssetGen tool, but could conceivably be used in other contexts.
	class tools_export tFxmlConverter : public FX::tFxFile
	{
	public:


	private:

		Fxml::tFile													mFxmlFile;
		tFilePathPtr												mResourcePath;

	public:

		tFxmlConverter( );
		~tFxmlConverter( );

		///
		/// \brief Load the fxml file.
		/// \return False if the file fails to load.
		b32 fLoadFxmlFile( const tFilePathPtr& fxmlFilePath, const tFilePathPtr& outputResourcePath );

		///
		/// \brief Convert platform-independent characteristics of the specified fxml file. This should
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
		/// \brief Get fxml file reference being converted.
		const Fxml::tFile& fFxmlFile( ) const { return mFxmlFile; }

		///
		/// \brief Get the resource output path.
		const tFilePathPtr& fGetResourcePath( ) const { return mResourcePath; }

	private:
		

	};
}

#endif//__tFxmlConverter__
