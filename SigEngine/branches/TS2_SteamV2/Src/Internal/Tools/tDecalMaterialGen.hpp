#ifndef __tDecalMaterialGen__
#define __tDecalMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a Decal material file.
	class tools_export tDecalMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tDecalMaterialGen, 0xE8BE8A62 );
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

		tFilePathPtr	mDiffuseMapPath;
		tUvParameters	mDiffuseUvParams;

		tFilePathPtr 	mNormalMapPath;
		tUvParameters	mNormalUvParams;
		f32			 	mBumpDepth;

		f32			 	mSpecSize;

	public:
		tDecalMaterialGen( );

	private:
		virtual const char* fGetMaterialName( ) const { return "Decal"; }
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

#endif//__tDecalMaterialGen__
