#include "BasePch.hpp"
#include "tSceneGraphFile.hpp"
#include "tTileCanvas.hpp"
#include "tResourceDepot.hpp"

namespace Sig
{

	define_lip_version( tSceneGraphFile, 16, 16, 16 );

	namespace
	{
		const char* gSceneGraphFileExts[]=
		{
			".mshb",
			".sigb",
		};

		const char* gSigmlExtensions[]=
		{
			".mshml",
			".sigml",
		};

		static_assert( array_length( gSceneGraphFileExts ) == array_length( gSigmlExtensions ) );
	}

	u32 tSceneGraphFile::fGetNumFileExtensions( )
	{
		return array_length( gSceneGraphFileExts );
	}

	const char* tSceneGraphFile::fGetFileExtension( u32 i )
	{
		sigassert( i < array_length( gSceneGraphFileExts ) );
		return gSceneGraphFileExts[ i ];
	}

	b32 tSceneGraphFile::fIsSceneGraphFile( const tFilePathPtr& path )
	{
		for( u32 i = 0; i < fGetNumFileExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( i ) ) )
				return true;
		return false;
	}

	u32 tSceneGraphFile::fGetNumSigmlFileExtensions( )
	{
		return array_length( gSigmlExtensions );
	}

	const char* tSceneGraphFile::fGetSigmlFileExtension( u32 i )
	{
		sigassert( i < array_length( gSigmlExtensions ) );
		return gSigmlExtensions[ i ];
	}

	b32 tSceneGraphFile::fIsSigmlFile( const tFilePathPtr& path )
	{
		for( u32 i = 0; i < fGetNumSigmlFileExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetSigmlFileExtension( i ) ) )
				return true;
		return false;
	}

	b32 tSceneGraphFile::fIsMshmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetSigmlFileExtension( 0 ) );
	}

	b32 tSceneGraphFile::fIsSigmlFileExclusive( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetSigmlFileExtension( 1 ) );
	}

	tFilePathPtr tSceneGraphFile::fSigmlPathToSigb( const tFilePathPtr& path )
	{
		sigassert( fGetNumSigmlFileExtensions( ) == tSceneGraphFile::fGetNumFileExtensions( ) );
		for( u32 i = 0; i < fGetNumSigmlFileExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), fGetSigmlFileExtension( i ) ) )
				return tFilePathPtr::fSwapExtension( path, tSceneGraphFile::fGetFileExtension( i ) );
		return path;
	}

	tFilePathPtr tSceneGraphFile::fSigbPathToSigml( const tFilePathPtr& path )
	{
		sigassert( fGetNumSigmlFileExtensions( ) == tSceneGraphFile::fGetNumFileExtensions( ) );
		for( u32 i = 0; i < fGetNumSigmlFileExtensions( ); ++i )
			if( StringUtil::fCheckExtension( path.fCStr( ), tSceneGraphFile::fGetFileExtension( i ) ) )
				return tFilePathPtr::fSwapExtension( path, fGetSigmlFileExtension( i ) );
		return path;
	}





	tSceneGraphFile::tSceneGraphFile( )
		: mSkeletonBinding( Math::tMat3f::cIdentity )
		, mSkeletonBindingInv( Math::tMat3f::cIdentity )
		, mDefaultLight( 0 )
		, mLODSettings( 0 )
		, mTilePackagePath( 0 )
		, mEntityData( 0 )
	{
		mBounds.fInvalidate( );
	}

	tSceneGraphFile::tSceneGraphFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, tEntityDefProperties( cNoOpTag )
		, mBounds( cNoOpTag )
		, mSkeletonBinding( cNoOpTag )
		, mSkeletonBindingInv( cNoOpTag )
		, mObjects( cNoOpTag )
		, mTilePackage( cNoOpTag )
	{
	}

	tSceneGraphFile::~tSceneGraphFile( )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			delete mObjects[ i ];

		delete mDefaultLight;
		delete mLODSettings;
		delete mEntityData;
	}

	void tSceneGraphFile::fGatherRuntimeResources( tResource& ownerResource )
	{
		if( tTileCanvasEntityDef::fTilesetOverride().fNull() && mTilePackagePath )
		{
			tResourceDepot* depot = ownerResource.fGetOwner( );
			sigassert( depot );
	
			mTilePackage.fTreatAsObject() = depot->fQuery( mTilePackagePath->fGetResourceId() );

			fAddRuntimeResourcePtr( mTilePackage.fTreatAsObject() );
		}
	}

	void tSceneGraphFile::fOnFileLoaded( const tResource& ownerResource )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[i]->fOnFileLoaded( );
	}

	void tSceneGraphFile::fOnSubResourcesLoaded( const tResource& ownerResource, b32 success )
	{
		if( !success )
			return;

		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			if( mObjects[i]->fOnSubResourcesLoaded( ownerResource ) )
			{
				// in case object has changed, update bounds
				if( mObjects[ i ]->fHasRenderableBounds( ) && mObjects[ i ]->mBounds.fIsValid( ) )
					mBounds |= mObjects[i]->mBounds;
			}
		}
	}

	void tSceneGraphFile::fOnFileUnloading( const tResource& ownerResource )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[i]->fOnFileUnloading( );
	}

	void tSceneGraphFile::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ]->fCollectEntities( params );
	}

}
