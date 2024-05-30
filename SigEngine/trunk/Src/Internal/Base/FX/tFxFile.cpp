#include "BasePch.hpp"
#include "tFxFile.hpp"
#include "tResource.hpp"

// other scene graph entitydef types
#include "FX/tParticleSystem.hpp"
#include "FX/tMeshSystem.hpp"
#include "FX/tAnimatedLightEntity.hpp"

namespace Sig
{
namespace FX
{
	register_rtti_factory( tBinaryF32Graph, true );
	register_rtti_factory( tBinaryV2Graph, true );
	register_rtti_factory( tBinaryV3Graph, true );
	register_rtti_factory( tBinaryV4Graph, true );

	register_rtti_factory( tParticleSystemDef, true )
	register_rtti_factory( tParticleAttractorDef, true )
	register_rtti_factory( tMeshSystemDef, true )
	register_rtti_factory( tAnimatedLightDef, true )
}
}


namespace Sig
{
namespace FX
{
	define_lip_version( tFxFile, 7, 7, 7 );

	namespace
	{
		const char* gFxmlBinaryExts[]=
		{
			".fxb",
		};

		const char* gFxmlFileExts[]=
		{
			".fxml",
		};

		static_assert( array_length( gFxmlBinaryExts ) == array_length( gFxmlFileExts ) );
	}

	u32 tFxFile::fGetNumBinaryExtensions( )
	{
		return array_length( gFxmlBinaryExts );
	}

	const char* tFxFile::fGetBinaryExtension( u32 i )
	{
		sigassert( i < array_length( gFxmlBinaryExts ) );
		return gFxmlBinaryExts[ i ];
	}

	b32 tFxFile::fIsBinary( const tFilePathPtr& path )
	{
		for( u32 i = 0; i < fGetNumBinaryExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetBinaryExtension( i ) ) )
				return true;
		return false;
	}

	u32 tFxFile::fGetNumFileExtensions( )
	{
		return array_length( gFxmlFileExts );
	}

	const char* tFxFile::fGetFileExtension( u32 i )
	{
		sigassert( i < array_length( gFxmlFileExts ) );
		return gFxmlFileExts[ i ];
	}

	b32 tFxFile::fIsFile( const tFilePathPtr& path )
	{
		for( u32 i = 0; i < fGetNumFileExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( i ) ) )
				return true;
		return false;
	}

	tFilePathPtr tFxFile::fFxmlPathToFxb( const tFilePathPtr& path )
	{
		sigassert( fGetNumBinaryExtensions( ) == fGetNumFileExtensions( ) );

		const char* extension = fGetBinaryExtension( );
		for( u32 i = 0; i < fGetNumBinaryExtensions( ); ++i )
		{
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetBinaryExtension( i ) ) )
			{
				extension = fGetBinaryExtension( i );
				break;
			}
		}

		return tFilePathPtr::fSwapExtension( path, extension );
	}

	tFilePathPtr tFxFile::fFxbPathToFxml( const tFilePathPtr& path )
	{
		sigassert( fGetNumBinaryExtensions( ) == fGetNumFileExtensions( ) );

		const char* extension = fGetFileExtension( );
		for( u32 i = 0; i < fGetNumFileExtensions( ); ++i )
		{
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( i ) ) )
			{
				extension = fGetFileExtension( i );
				break;
			}
		}

		return tFilePathPtr::fSwapExtension( path, extension );
	}


	tFxFile::tFxFile( )
		: mLifetime( 0.f )
		, mFlags( 0 )
	{

	}

	tFxFile::tFxFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mObjects( cNoOpTag )
	{
	}

	tFxFile::~tFxFile( )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			delete mObjects[i];
	}

	void tFxFile::fOnFileLoaded( const tResource& ownerResource )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[i]->fOnFileLoaded( );
	}

	void tFxFile::fOnSubResourcesLoaded( const tResource& ownerResource, b32 success )
	{
		if( !success )
			return;

		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			if( mObjects[i]->fOnSubResourcesLoaded( ownerResource ) )
			{
				// in case object has changed, update bounds
				if( mObjects[i]->mBounds.fIsValid( ) )
					mBounds |= mObjects[i]->mBounds;
			}
		}
	}

	void tFxFile::fOnFileUnloading( const tResource& ownerResource )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[i]->fOnFileUnloading( );
	}

	void tFxFile::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[i]->fCollectEntities( params );
	}

}
}

