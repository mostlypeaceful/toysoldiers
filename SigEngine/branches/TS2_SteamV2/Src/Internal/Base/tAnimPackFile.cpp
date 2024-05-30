#include "BasePch.hpp"
#include "tAnimPackFile.hpp"

namespace Sig
{

	///
	/// \section tAnimPackFile
	///

	const u32 tAnimPackFile::cVersion = 3;

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
		mAnimsByName.fConstruct( 2 * mAnims.fCount( ) + 1 );

		// initialize the anim map (i.e., insert all anims)
		tAnimMap& animMap = mAnimsByName.fTreatAsObject( );
		for( u32 i = 0; i < mAnims.fCount( ); ++i )
			animMap.fInsert( mAnims[ i ].mName->fGetStringPtr( ).fGetHashValue( ), &mAnims[ i ] );
	}

	void tAnimPackFile::fOnFileUnloading( )
	{
		mAnimsByName.fDestroy( );
	}

	const tKeyFrameAnimation* tAnimPackFile::fFindAnim( const tStringPtr& name ) const 
	{
		const tKeyFrameAnimation** find = fGetAnimMap( ).fFind( name.fGetHashValue( ) );
		return find ? *find : 0;
	}

}


namespace Sig
{
	namespace
	{
		static tKeyFrameAnimation* fFindAnim( const tAnimPackFile* packFile, const tStringPtr& animName )
		{
			tKeyFrameAnimation* found = ( tKeyFrameAnimation* )packFile->fFindAnim( animName );

			if( !found )
				log_warning( Log::cFlagAnimation, "Couldn't find animation [" << animName << "] in anim pack." );

			return found;
		}

		static tKeyFrameAnimation* fFindAnimNoWarn( const tAnimPackFile* packFile, const tStringPtr& animName )
		{
			return ( tKeyFrameAnimation* )packFile->fFindAnim( animName );
		}
	}
	void tAnimPackFile::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAnimPackFile, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Find"), &::Sig::fFindAnim)
			.GlobalFunc(_SC("FindNoWarn"), &::Sig::fFindAnimNoWarn)
			;
		vm.fNamespace(_SC("Anim")).Bind(_SC("PackFile"), classDesc);
	}
}

