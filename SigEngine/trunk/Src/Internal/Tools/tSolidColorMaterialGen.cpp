#include "ToolsPch.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "tSolidColorMaterialGen.hpp"

namespace Sig
{
	register_rtti_factory( tSolidColorMaterialGen, false );

	namespace
	{
		void fVs30( std::string& o )
		{
			std::stringstream ss;
			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float3x4 gWorld     : register( c" << Gfx::tSolidColorMaterial::cVSLocalToWorld << " );" << std::endl;
			ss << "float4x4 gProjView  : register( c" << Gfx::tSolidColorMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "float4 gWorldEyePos : register( c" << Gfx::tSolidColorMaterial::cVSWorldEyePos << " );" << std::endl;
			ss << "struct tOutput {" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "  float4 vOutColor : COLOR0;" << std::endl;
			ss << "};" << std::endl;
			ss << "tOutput main( float3 vInPos : POSITION, float4 vInColor : COLOR ) {" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "   o.vOutColor = vInColor;" << std::endl;
			ss << "   float3 vInPosWorld = mul( gWorld, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   o.vCP = mul( gProjView, float4( vInPosWorld, 1.f ) );" << std::endl;			
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fPs30( std::string& o )
		{
			std::stringstream ss;
			ss << "float4 gTint : register( c" << Gfx::tSolidColorMaterial::cPSRgbaTint << " );" << std::endl;
			ss << "void main( in float4 vInColor : COLOR0, out float4 oColor : COLOR ) {" << std::endl;
			ss << "   oColor = gTint * vInColor;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSolidColorMaterialGen& o )
	{
	}

	void tSolidColorMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tSolidColorMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tSolidColorMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tSolidColorMaterial* mtl = new Gfx::tSolidColorMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tSolidColorMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tSolidColorMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return tMaterialGenBase::fIsEquivalent( other );
	}

	tFilePathPtr tSolidColorMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tSolidColorMaterial::fMaterialFilePath( );
	}

	void tSolidColorMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tSolidColorMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tSolidColorMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 1 ); mtlFile.mShaderLists.fFront( ).fNewArray( Gfx::tSolidColorMaterial::cShaderSlotCount );
		tShaderBufferSet shaderBuffers( 1 ); shaderBuffers.fFront( ).fNewArray( Gfx::tSolidColorMaterial::cShaderSlotCount );

		std::string vs, ps;
		fVs30( vs );
		fPs30( ps );

		fAddVertexShader( pid, 0, Gfx::tSolidColorMaterial::cShaderSlotVS, mtlFile, shaderBuffers, vs );
		fAddPixelShader( pid, 0, Gfx::tSolidColorMaterial::cShaderSlotPS, mtlFile, shaderBuffers, ps );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tSolidColorMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tSolidColorMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tSolidColorMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

