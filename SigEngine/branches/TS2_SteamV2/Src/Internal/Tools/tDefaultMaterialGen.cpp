#include "ToolsPch.hpp"
#include "Gfx/tDefaultMaterial.hpp"
#include "tDefaultMaterialGen.hpp"

namespace Sig
{
	register_rtti_factory( tDefaultMaterialGen, false );

	namespace
	{
		void fVs30( std::string& o )
		{
			std::stringstream ss;
			ss << "#pragma pack_matrix( row_major )" << std::endl;
			ss << "float3x4 gWorld     : register( c" << Gfx::tDefaultMaterial::cVSLocalToWorld << " );" << std::endl;
			ss << "float4x4 gProjView  : register( c" << Gfx::tDefaultMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "float4 gWorldEyePos : register( c" << Gfx::tDefaultMaterial::cVSWorldEyePos << " );" << std::endl;
			ss << "struct tOutput {" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "  float4 vOutColor : COLOR0;" << std::endl;
// TODODELETE
//ss << "  float2 vOutDepth : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;
			ss << "tOutput main( float3 vInPos : POSITION, float3 vInNormal : NORMAL, float4 vInColor : COLOR ) {" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "   float3 vInNormalWorld = mul( (float3x3)gWorld, vInNormal );" << std::endl;
			ss << "   o.vOutColor = 0.3f + 0.5f * saturate( dot( vInNormalWorld, normalize( float3( 0.f, 1.f, 0.f ) ) ) );" << std::endl;
			ss << "   o.vOutColor *= vInColor;" << std::endl;
			ss << "   float3 vInPosWorld = mul( gWorld, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   o.vCP = mul( gProjView, float4( vInPosWorld, 1.f ) );" << std::endl;
// TODODELETE
//ss << "   o.vCP.z = o.vCP.z * o.vCP.w;" << std::endl; // linearize depth
//ss << "   o.vOutDepth.xy = float2( o.vCP.z * o.vCP.w / gWorldEyePos.w, o.vCP.w );" << std::endl;
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fPs30( std::string& o )
		{
			std::stringstream ss;
			ss << "void main( in float4 vInColor : COLOR0, out float4 oColor : COLOR ) {" << std::endl;
// TODODELETE
//ss << "   oDepth = float4( vInDepth.x / vInDepth.y, 0.f, 0.f, 0.f );" << std::endl;
			ss << "   oColor = float4( vInColor.rgb, 1 );" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tDefaultMaterialGen& o )
	{
	}

	void tDefaultMaterialGen::fSerialize( tXmlSerializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tDefaultMaterialGen::fSerialize( tXmlDeserializer& s ) { tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tDefaultMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tDefaultMaterial* mtl = new Gfx::tDefaultMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tDefaultMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tDefaultMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return tMaterialGenBase::fIsEquivalent( other );
	}

	tFilePathPtr tDefaultMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tDefaultMaterial::fMaterialFilePath( );
	}

	void tDefaultMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tDefaultMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tDefaultMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 1 ); mtlFile.mShaderLists.fFront( ).fNewArray( Gfx::tDefaultMaterial::cShaderSlotCount );
		tShaderBufferSet shaderBuffers( 1 ); shaderBuffers.fFront( ).fNewArray( Gfx::tDefaultMaterial::cShaderSlotCount );

		std::string vs, ps;
		fVs30( vs );
		fPs30( ps );

		fAddVertexShader( pid, 0, Gfx::tDefaultMaterial::cShaderSlotVS, mtlFile, shaderBuffers, vs );
		fAddPixelShader( pid, 0, Gfx::tDefaultMaterial::cShaderSlotPS, mtlFile, shaderBuffers, ps );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tDefaultMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tDefaultMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tDefaultMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

