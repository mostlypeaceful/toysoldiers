#include "ToolsPch.hpp"
#include "Gfx/tFontMaterial.hpp"
#include "Gfx/tTextureFile.hpp"
#include "tFontMaterialGen.hpp"
#include "tTextureGen.hpp"

namespace Sig
{
	register_rtti_factory( tFontMaterialGen, false );

	namespace
	{
		void fVs( std::string& o )
		{
			std::stringstream ss;
			ss << "#pragma pack_matrix(row_major)" << std::endl;
			ss << "float3x4 gWorld     : register( c" << Gfx::tFontMaterial::cVSLocalToWorld << " );" << std::endl;
			ss << "float4x4 gProjView  : register( c" << Gfx::tFontMaterial::cVSWorldToProj << " );" << std::endl;
			ss << "struct tOutput {" << std::endl;
			ss << "  float4 vOutPos : POSITION;" << std::endl;
			ss << "  float2 vOutUv : TEXCOORD0;" << std::endl;
			ss << "};" << std::endl;
			ss << "tOutput main( float3 vInPos : POSITION, float2 vInUv : TEXCOORD0 ) {" << std::endl;
			ss << "   tOutput o;" << std::endl;
			ss << "   float3 vInPosWorld = mul( gWorld, float4( vInPos, 1.f ) );" << std::endl;
			ss << "   o.vOutPos = mul( gProjView, float4( vInPosWorld, 1.f ) );" << std::endl;
			ss << "   o.vOutUv = vInUv;" << std::endl;
			ss << "   return o;" << std::endl;
			ss << "}" << std::endl;
			o = ss.str( );
		}

		void fPsHeader( std::stringstream& ss, b32 outline )
		{
			ss << "float4 gTint : register( c0 );" << std::endl;
			ss << "sampler2D gFontMap : register( s0 );" << std::endl;
			ss << "float4 main( float2 uv : TEXCOORD0 ) : COLOR0 {" << std::endl;
			if( outline )
			{
				ss << "    float sample = tex2D( gFontMap, uv ).a;" << std::endl;
				ss << "    float color = sample * sample;" << std::endl;
				ss << "    float alpha = sample;" << std::endl;
				ss << "    return float4( gTint.rgb * color, gTint.a * alpha );" << std::endl;
			}
			else
				ss << "    return float4( gTint.rgb, gTint.a * tex2D( gFontMap, uv ).a );" << std::endl;
			ss << "}" << std::endl;
		}

		void fPsStandard( std::string& o )
		{
			std::stringstream ss;
			fPsHeader( ss, false );
			o = ss.str( );
		}

		void fPsOutline( std::string& o )
		{
			std::stringstream ss;
			fPsHeader( ss, true );
			o = ss.str( );
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFontMaterialGen& o )
	{
	}

	void tFontMaterialGen::fSerialize( tXmlSerializer& s )	{ tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }
	void tFontMaterialGen::fSerialize( tXmlDeserializer& s ) { tMaterialGenBase::fSerialize( s ); fSerializeXmlObject( s, *this ); }

	tFontMaterialGen::tFontMaterialGen( )
		: mOutline( false )
	{
	}

	Gfx::tMaterial* tFontMaterialGen::fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned )
	{
		return fCreateFontMaterial( lipFileCreator );
	}

	Gfx::tFontMaterial* tFontMaterialGen::fCreateFontMaterial( tLoadInPlaceFileBase& lipFileCreator )
	{
		Gfx::tFontMaterial* mtl = new Gfx::tFontMaterial( );

		// set base render state variables before converting base
		mTransparency = true;
		mAlphaCutOut = 0;
		mTwoSided = true;

		// convert base (render states)
		fConvertBase( mtl );

		// store material file resource pointer
		const tResourceId mtlFileRid = tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tFontMaterial::fMaterialFilePath( ) );
		mtl->fSetMaterialFileResourcePtrUnOwned( lipFileCreator.fAddLoadInPlaceResourcePtr( mtlFileRid ) );

		// store font map texture resource pointer
		const tFilePathPtr binaryPath = tTextureGen::fCreateResourceNameFromInputPath( mFontMap );
		const tResourceId rid = tResourceId::fMake<Gfx::tTextureFile>( binaryPath );
		mtl->mFontMap = lipFileCreator.fAddLoadInPlaceResourcePtr( rid );

		// store which shaders to use
		mtl->mVsSlot = Gfx::tFontMaterial::cVSStandard;
		if( mOutline )
			mtl->mPsSlot = Gfx::tFontMaterial::cPSOutline;
		else
			mtl->mPsSlot = Gfx::tFontMaterial::cPSStandard;

		return mtl;
	}

	b32 tFontMaterialGen::fIsEquivalent( const Sigml::tMaterial& other ) const
	{
		if( !tMaterialGenBase::fIsEquivalent( other ) )
			return false;

		const tFontMaterialGen& otherMyType = static_cast<const tFontMaterialGen&>( other );

		if( mFontMap != otherMyType.mFontMap )
			return false;

		if( mOutline != otherMyType.mOutline )
			return false;

		return true;
	}

	tFilePathPtr tFontMaterialGen::fMaterialFilePath( ) const
	{
		return Gfx::tFontMaterial::fMaterialFilePath( );
	}

	void tFontMaterialGen::fGenerateMaterialFileWii( tPlatformId pid )
	{
	}

	void tFontMaterialGen::fGenerateMaterialFilePcDx9( tPlatformId pid )
	{
		Gfx::tMaterialFile mtlFile;
		mtlFile.mMaterialCid = Rtti::fGetClassId<Gfx::tFontMaterial>( );
		mtlFile.mDiscardShaderBuffers = true; // don't need to keep them around on the pc
		mtlFile.mShaderLists.fNewArray( 1 ); mtlFile.mShaderLists.fFront( ).fNewArray( Gfx::tFontMaterial::cShaderSlotCount );
		tShaderBufferSet shaderBuffers( 1 ); shaderBuffers.fFront( ).fNewArray( Gfx::tFontMaterial::cShaderSlotCount );

		std::string vs, psStandard, psOutline;
		fVs( vs );
		fPsStandard( psStandard );
		fPsOutline( psOutline );

		fAddVertexShader( pid, 0, Gfx::tFontMaterial::cVSStandard, mtlFile, shaderBuffers, vs );
		fAddPixelShader( pid, 0, Gfx::tFontMaterial::cPSStandard, mtlFile, shaderBuffers, psStandard );
		fAddPixelShader( pid, 0, Gfx::tFontMaterial::cPSOutline, mtlFile, shaderBuffers, psOutline );

		fSaveMaterialFile( mtlFile, shaderBuffers, pid );
	}

	void tFontMaterialGen::fGenerateMaterialFilePcDx10( tPlatformId pid )
	{
	}

	void tFontMaterialGen::fGenerateMaterialFileXbox360( tPlatformId pid )
	{
		fGenerateMaterialFilePcDx9( pid );
	}

	void tFontMaterialGen::fGenerateMaterialFilePs3Ppu( tPlatformId pid )
	{
	}

}

