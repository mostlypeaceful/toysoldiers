#include "ToolsPch.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "tFullBrightMaterialGen.hpp"

namespace Sig
{
	register_rtti_factory( tFullBrightMaterialGen, false );

	namespace
	{
		void fVs30( std::string& o )
		{
			std::stringstream ss;
			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float3x4 gWorld     : register( c" << Gfx::tFullBrightMaterial::cVSLocalToWorld << " );" << std::endl;
			ss << "float4x4 gProjView  : register( c" << Gfx::tFullBrightMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "float4 gWorldEyePos : register( c" << Gfx::tFullBrightMaterial::cVSWorldEyePos << " );" << std::endl;
			ss << "struct tOutput {" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "  float4 vOutColor : COLOR0;" << std::endl;
			ss << "  float2 vOutUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;
			ss << "tOutput main( float3 vInPos : POSITION, float4 vInColor : COLOR, float4 vInUv : TEXCOORD0 ) {" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "   o.vOutColor = vInColor;" << std::endl;
			ss << "   float3 vInPosWorld = mul( gWorld, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   o.vCP = mul( gProjView, float4( vInPosWorld, 1.f ) );" << std::endl;			
			ss << "   o.vOutUv.xy = vInUv;" << std::endl;
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fPs30( std::string& o )
		{
			std::stringstream ss;
			ss << "sampler2D gColorMap : register( s" << 0 << " );" << std::endl;
			ss << "float4 gTint : register( c" << Gfx::tFullBrightMaterial::cPSRgbaTint << " );" << std::endl;
			ss << "void main( in float4 vInColor : COLOR0, in float2 vInUv : TEXCOORD0, out float4 oColor : COLOR ) {" << std::endl;
			ss << "   oColor = gTint * vInColor * tex2D( gColorMap, vInUv );" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFullBrightMaterialGen& o )
	{
	}

	void tFullBrightMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tFullBrightMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tFullBrightMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tFullBrightMaterial* mtl = new Gfx::tFullBrightMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tFullBrightMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tFullBrightMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return tMaterialGenBase::fIsEquivalent( other );
	}

	tFilePathPtr tFullBrightMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tFullBrightMaterial::fMaterialFilePath( );
	}

	void tFullBrightMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tFullBrightMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tFullBrightMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 1 ); mtlFile.mShaderLists.fFront( ).fNewArray( Gfx::tFullBrightMaterial::cShaderSlotCount );
		tShaderBufferSet shaderBuffers( 1 ); shaderBuffers.fFront( ).fNewArray( Gfx::tFullBrightMaterial::cShaderSlotCount );

		std::string vs, ps;
		fVs30( vs );
		fPs30( ps );

		fAddVertexShader( pid, 0, Gfx::tFullBrightMaterial::cShaderSlotVS, mtlFile, shaderBuffers, vs );
		fAddPixelShader( pid, 0, Gfx::tFullBrightMaterial::cShaderSlotPS, mtlFile, shaderBuffers, ps );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tFullBrightMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tFullBrightMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tFullBrightMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

