#ifndef __tPhongMaterialGen__
#define __tPhongMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a Phong material file.
	class tools_export tPhongMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tPhongMaterialGen, 0x310518F3 );
	public:

		struct tUvParameters
		{
			Math::tVec2u mMirrorUv;
			Math::tVec2u mWrapUv;
			Math::tVec2f mRepeatUv;
			Math::tVec2f mOffsetUv;
			std::string  mUvSetName;

			tUvParameters( )
				: mMirrorUv(false, false)
				, mWrapUv(true, true)
				, mRepeatUv(1.f, 1.f)
				, mOffsetUv(0.f, 0.f)
				, mUvSetName("")
			{
			}

			inline b32 operator==( const tUvParameters& other ) const
			{
				return	mMirrorUv.fEqual( other.mMirrorUv ) &&
						mWrapUv.fEqual( other.mWrapUv ) &&
						mRepeatUv.fEqual( other.mRepeatUv ) &&
						mOffsetUv.fEqual( other.mOffsetUv ) &&
						mUvSetName == other.mUvSetName;
			}
			inline b32 operator!=( const tUvParameters& other ) const
			{
				return !operator==( other );
			}
		};

		Math::tVec4f	mDiffuseColor;
		tFilePathPtr	mDiffuseMapPath;
		tUvParameters	mDiffuseUvParams;

		Math::tVec4f 	mSpecColor;
		tFilePathPtr 	mSpecColorMapPath;
		tUvParameters	mSpecColorUvParams;

		Math::tVec4f 	mEmissiveColor;
		tFilePathPtr 	mEmissiveMapPath;
		tUvParameters	mEmissiveUvParams;

		Math::tVec4f 	mOpacityColor;
		tFilePathPtr 	mOpacityMapPath;
		tUvParameters	mOpacityUvParams;

		tFilePathPtr 	mNormalMapPath;
		tUvParameters	mNormalUvParams;
		f32			 	mBumpDepth;

		f32			 	mSpecSize;

	public:
		tPhongMaterialGen( );

	private:
		virtual const char* fGetMaterialName( ) const { return "Phong"; }
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned );
		virtual b32	 fIsEquivalent( const Sigml::tMaterial& other ) const;
		virtual void fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut );
		virtual tFilePathPtr fMaterialFilePath( ) const;
		virtual void fGenerateMaterialFileWii( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx9( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx10( tPlatformId pid );
		virtual void fGenerateMaterialFileXbox360( tPlatformId pid );
		virtual void fGenerateMaterialFilePs3Ppu( tPlatformId pid );

		b32 fAddTexture( 
			Math::tVec4f& color, 
			tLoadInPlaceResourcePtr*& resourcePtr, 
			Math::tVec4f& uvXform, 
			const Math::tVec4f& inColor,
			const tFilePathPtr& path,
			const tUvParameters& uvParams, 
			tLoadInPlaceFileBase& lipFileCreator );
	};

}

#endif//__tPhongMaterialGen__
