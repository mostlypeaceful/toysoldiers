#include "BasePch.hpp"
#include "tAnimPackFile.hpp"

namespace Sig
{

	///
	/// \section tAnimPackFile
	///

	define_lip_version( tAnimPackFile, 3, 3, 3 );

	const char*	tAnimPackFile::fGetFileExtension( )
	{
		return ".anib";
	}

	tFilePathPtr tAnimPackFile::fAnipkPathToAnib( const char* anipkPath )
	{
		return fAnipkPathToAnib( tFilePathPtr( anipkPath ) );
	}
	tFilePathPtr tAnimPackFile::fAnipkPathToAnib( const tFilePathPtr& anipkPath )
	{
		return tFilePathPtr::fSwapExtension( anipkPath, fGetFileExtension( ) );
	}

	tAnimPackFile::tAnimPackFile( )
		: mSkeletonResource( 0 )
	{
	}

	tAnimPackFile::tAnimPackFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mAnims( cNoOpTag )
		, mAnimsByName( cNoOpTag )
	{
	}

	void tAnimPackFile::fOnFileLoaded( const tResource& ownerResource )
	{
		// construct anim map with twice as many entries as
		// there are anims, just to maintain a fairly sparse harsh table
		mAnimsByName.fTreatAsObject( ).fSetCapacity( 2 * mAnims.fCount( ) + 1 );

		// initialize the anim map (i.e., insert all anims)
		tAnimMap& animMap = mAnimsByName.fTreatAsObject( );
		for( u32 i = 0; i < mAnims.fCount( ); ++i )
			animMap.fInsert( mAnims[ i ].mName->fGetStringPtr( ).fGetHashValue( ), &mAnims[ i ] );
	}

	void tAnimPackFile::fOnFileUnloading( const tResource& ownerResource )
	{
		mAnimsByName.fDestroy( );
	}

	const tKeyFrameAnimation* tAnimPackFile::fFindAnim( const tStringPtr& name ) const 
	{
		const tKeyFrameAnimation** find = fGetAnimMap( ).fFind( name.fGetHashValue( ) );
		log_sigcheckfail( find, "Failed to tAnimPackFile::fFindAnim [" << name << "]", return NULL );
		return *find;
	}

	s32 tAnimPackFile::fIndexOfAnim( const tStringPtr& name ) const
	{
		const tKeyFrameAnimation* anim = fFindAnim( name );
		return anim ? anim - mAnims.fBegin( ) : -1;
	}

	s32 tAnimPackFile::fIndexOfAnim( const tKeyFrameAnimation& anim ) const
	{
		const s32 idx = &anim - mAnims.fBegin( );
		return ( idx >= 0 && idx < ( s32 )mAnims.fCount( ) ) ? idx : -1;
	}

	u32 tAnimPackFile::fComputeStorage( std::string& display ) const
	{
		u32 numBytes = fGetAnimMap( ).fGetCapacity( ) * sizeof( tAnimMap::tMyEntry );
		
		tAnimMap::tConstIterator ptr = fGetAnimMap( ).fBegin( );
		const tAnimMap::tConstIterator end = fGetAnimMap( ).fEnd( );
		for( ; ptr != end; ++ptr )
		{
			if( ptr->fNullOrRemoved( ) )
				continue;

			numBytes += ptr->mValue->fComputeStorage( );
		}

		std::stringstream ss;
		ss << std::fixed << std::setprecision( 2 ) << Memory::fToMB<f32>( numBytes ) << "MB";
		display = ss.str( );

		return numBytes;
	}

}


namespace Sig
{
	namespace
	{
		static tKeyFrameAnimation* fFindAnim( const tAnimPackFile* packFile, const tStringPtr& animName )
		{
			sigcheckfail( packFile, return NULL );
			const tKeyFrameAnimation* found = packFile->fFindAnim( animName );
			return const_cast<tKeyFrameAnimation*>( found ); //squirrel doesn't handle const.
		}
	}
	void tAnimPackFile::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAnimPackFile, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Find"), &::Sig::fFindAnim)
			;
		vm.fNamespace(_SC("Anim")).Bind(_SC("PackFile"), classDesc);
	}
}

