#ifndef __tSigmlConverter__
#define __tSigmlConverter__
#include "Sigml.hpp"
#include "tSceneGraphFile.hpp"
#include "tTextureSysRam.hpp"
#include "tMesh.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryBufferVRam.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"
#include "Gfx/tIndexBufferSysRam.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	namespace Gfx { class tMaterial; }

	class tMesh;
	class tTextureSysRam;

	///
	/// \brief Converts Sigml files to the binary/game equivalent, Sigb files.
	/// Used in AssetGen tool, but could conceivably be used in other contexts.
	class tools_export tSigmlConverter : public tSceneGraphFile
	{
	public:

		struct tIndexBufferData
		{
			u32							mNumTris;
			u32							mTriOffset;
			Gfx::tIndexBufferSysRam		mIndexBuffer;

			tIndexBufferData( ) : mNumTris( 0 ), mTriOffset( 0 ) { }
		};

		struct tMeshData
		{
			tFilePathPtr								mResourcePath;
			tGrowableArray<Gfx::tGeometryBufferSysRam>	mGeometryData;
			tGrowableArray<tIndexBufferData>			mIndexData;
		};

		struct tTextureData
		{
			tFilePathPtr						mResourcePath;
			tTextureSysRam						mTextureObject;
		};

		struct tMaterialData
		{
			tStrongPtr<Gfx::tMaterial> mSkinned, mNonSkinned;
		};

	private:

		Sigml::tFile												mSigmlFile;
		tFilePathPtr												mResourcePath;
		tGrowableArray< tStrongPtr<tMesh> >							mMeshes;
		tGrowableArray< tStrongPtr<tMeshData> >						mMeshData;
		tGrowableArray< tStrongPtr<Gfx::tVertexFormatVRam> >		mSubMeshVertexFormats;
		tGrowableArray< tMaterialData >								mMaterials;
		tGrowableArray< tStrongPtr<tTextureData> >					mTextures;

	public:

		tSigmlConverter( );
		~tSigmlConverter( );

		///
		/// \brief Load the sigml file.
		/// \return False if the file fails to load.
		b32 fLoadSigmlFile( const tFilePathPtr& sigmlFilePath, const tFilePathPtr& skinFilePath, const tFilePathPtr& outputResourcePath );

		///
		/// \brief Convert platform-independent characteristics of the specified sigml file. This should
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
		const Sigml::tFile& fSigmlFile( ) const { return mSigmlFile; }

		///
		/// \brief Get mesh pointer.
		inline const tStrongPtr<tMesh>& fGetMesh( u32 ithMesh ) const { return mMeshes[ ithMesh ]; }

		///
		/// \brief Add mesh data.
		inline void fAddMeshData( const tStrongPtr<tMeshData>& meshData ) { mMeshData.fPushBack( meshData ); }

		///
		/// \brief Get the current mesh data count.
		inline u32 fGetMeshDataCount( ) const { return mMeshData.fCount( ); }

		///
		/// \brief Add a texture object that will 
		inline void fAddTextureObject( const tStrongPtr<tTextureData>& tex ) { mTextures.fPushBack( tex ); }

		///
		/// \brief Get the number of texture objects.
		inline u32 fGetTextureObjectCount( ) const { return mTextures.fCount( ); }

		///
		/// \brief Get the resource output path.
		const tFilePathPtr& fGetResourcePath( ) const { return mResourcePath; }

	private:
		void fOutputGeometryFile( const tMeshData& meshData, tPlatformId pid );
		void fOutputTextureFile( const tTextureData& texData, tPlatformId pid );
		void fConvertRayCastSubMeshData( u32 ithSubMesh, tMesh& gfxMesh, const Sigml::tMeshPtr& sigmlMesh );
		void fConvertRayCastMeshData( tMesh& gfxMesh, const Sigml::tMeshPtr& sigmlMesh );
		b32  fConvertMesh( tMesh& gfxMesh, tMeshData& meshData, const Sigml::tMeshPtr& sigmlMesh, u32& ithVb, u32& ithIb );
	};
}

#endif//__tSigmlConverter__
