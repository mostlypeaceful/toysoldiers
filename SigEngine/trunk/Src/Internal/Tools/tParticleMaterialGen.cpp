#include "ToolsPch.hpp"
#include "Gfx/tParticleMaterial.hpp"
#include "tParticleMaterialGen.hpp"

namespace Sig
{
	register_rtti_factory( tParticleMaterialGen, false );

	namespace
	{
		void fVs30( std::string& o )
		{
			std::stringstream ss;

			ss << "#pragma pack_matrix( row_major )" << std::endl;

			ss << "float3x4 gWorldView : register( c" << Gfx::tParticleMaterial::cVSLocalToView<< " );" << std::endl;
			ss << "float4x4 gProj : register( c" << Gfx::tParticleMaterial::cVSViewToProj << " );" << std::endl;
			ss << "float4 gWorldEyePos : register( c" << Gfx::tParticleMaterial::cVSWorldEyePos << " );" << std::endl;

			ss << "struct tOutput" << std::endl;
			ss << "{" << std::endl;
			ss << "  float4 vCP : POSITION;" << std::endl;
			ss << "  float4 vOutColor : COLOR0;" << std::endl;
			ss << "  float2 vUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;

			ss << "tOutput main( float3 vInPos : POSITION, float4 vInColor : COLOR, float4 vCorners : TEXCOORD0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   tOutput o;" << std::endl;

			ss << "   o.vOutColor = vInColor;" << std::endl;

			ss << "   float xSign = sign( vCorners.x );" << std::endl;
			ss << "   float ySign = sign( vCorners.y );" << std::endl;
			ss << "   float roll = vCorners.z;" << std::endl;

			ss << "   float sinRoll;" << std::endl;
			ss << "   float cosRoll;" << std::endl;
			ss << "   sincos( roll, sinRoll, cosRoll );" << std::endl;

			ss << "   float2 uv = float2( xSign * 0.5f + 0.5f, 1.0f - (ySign * 0.5f + 0.5f) );" << std::endl;
			ss << "   o.vUv = uv;" << std::endl;

			ss << "   float3 vInPosView = mul( gWorldView, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   float3 cameraX = vCorners.x * float3( cosRoll, -sinRoll, 0 );" << std::endl;
			ss << "   float3 cameraY = vCorners.y * float3( sinRoll, cosRoll, 0 );" << std::endl;

			ss << "   float3 vInPosBillboard = vInPosView + cameraX + cameraY;" << std::endl;
			ss << "   o.vCP = mul( gProj, float4( vInPosBillboard, 1.f ) );" << std::endl;			

			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
		}

		void fPs30( std::string& o )
		{
			std::stringstream ss;

			//ss << "sampler2D gEmissiveMap : register( s" << 0 << " );" << std::endl;
			ss << "float4 gTint : register( c" << Gfx::tParticleMaterial::cPSRgbaTint << " );" << std::endl;

			ss << "void main( in float4 vInColor : COLOR0, in float2 vInUv : TEXCOORD0, out float4 oColor : COLOR0 )" << std::endl;
			ss << "{" << std::endl;
			ss << "   float x = ( vInUv.x - 0.5f ) * 2.0f;" << std::endl;
			ss << "   float y = ( vInUv.y - 0.5f ) * 2.0f;" << std::endl;
			ss << "   float dist = 1.0f - ( x*x + y*y );" << std::endl;
			ss << "   oColor = gTint * vInColor * float4( 1.0, 1.0, 1.0, dist );" << std::endl;//gTint * vInColor * tex2D( gEmissiveMap, vInUv );" << std::endl;
			ss << "}" << std::endl;

			o = ss.str( );
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tParticleMaterialGen& o )
	{
	}

	void tParticleMaterialGen::fSerialize( tXmlSerializer& s )		{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tParticleMaterialGen::fSerialize( tXmlDeserializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	Gfx::tMaterial* tParticleMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		Gfx::tParticleMaterial* mtl = new Gfx::tParticleMaterial( );

		fConvertBase( mtl );

		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tParticleMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		return mtl;
	}

	b32 tParticleMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		return tMaterialGenBase::fIsEquivalent( other );
	}

	tFilePathPtr tParticleMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tParticleMaterial::fMaterialFilePath( );
	}

	void tParticleMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tParticleMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tParticleMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 1 ); mtlFile.mShaderLists.fFront( ).fNewArray( Gfx::tParticleMaterial::cShaderSlotCount );
		tShaderBufferSet shaderBuffers( 1 ); shaderBuffers.fFront( ).fNewArray( Gfx::tParticleMaterial::cShaderSlotCount );

		std::string vs, ps;
		fVs30( vs );
		fPs30( ps );

		fAddVertexShader( pid, 0, Gfx::tParticleMaterial::cShaderSlotVS, mtlFile, shaderBuffers, vs );
		fAddPixelShader( pid, 0, Gfx::tParticleMaterial::cShaderSlotPS, mtlFile, shaderBuffers, ps );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tParticleMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tParticleMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tParticleMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

