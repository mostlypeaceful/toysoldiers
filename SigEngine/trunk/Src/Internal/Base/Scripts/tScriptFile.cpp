#include "BasePch.hpp"
#include "tScriptFile.hpp"

namespace Sig
{
	namespace { static const Sqrat::Function cNullScriptObj; }

	define_lip_version( tScriptFile, 1, 1, 1 );

	const char*		tScriptFile::fGetFileExtension( ) { return ".nutb"; }

	tScriptFile::tScriptFile( )
		: mFlags( 0 )
		, mUniqueId( 0 )
		, mScriptClass( 0 )
	{
		fZeroOut( mStandardExportedFunctions );
	}
	tScriptFile::tScriptFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mByteCode( cNoOpTag )
		, mScriptImports( cNoOpTag )
		, mExportedFunctions( cNoOpTag )
		, mExportedVariables( cNoOpTag )
		, mStandardExportedFunctions( cNoOpTag )
	{
	}
	void tScriptFile::fInitScriptObjects( tScriptVm& vm )
	{
		for( u32 i = 0; i < mExportedFunctions.fCount( ); ++i )
			mExportedFunctions[ i ].mScriptObject.fTreatAsObject( ) = vm.fRootTable( ).GetFunction( mExportedFunctions[ i ].mCallableName->fGetStringPtr( ).fCStr( ) );
		for( u32 i = 0; i < mStandardExportedFunctions.fCount( ); ++i )
		{
			if( mStandardExportedFunctions[ i ].mCallableName )
				mStandardExportedFunctions[ i ].mScriptObject.fTreatAsObject( ) = vm.fRootTable( ).GetFunction( mStandardExportedFunctions[ i ].mCallableName->fGetStringPtr( ).fCStr( ) );
			else
				mStandardExportedFunctions[ i ].mScriptObject.fTreatAsObject( ) = cNullScriptObj;
		}
	}
	void tScriptFile::fOnSubResourcesLoaded( const tResource& ownerResource, b32 success )
	{
		if( !success )
			return;

// TODO REFACTOR need to resolve the fact that scripts are dependent on game-specific code registration - 
		// that said currently scripts aren't used in tools, but it would nice to be able to do so
#ifdef target_game
		tScriptVm::fInstance( ).fRegisterScriptFile( *this, ownerResource );
		fInitScriptObjects( tScriptVm::fInstance( ) );
#endif//target_game
	}
	void tScriptFile::fOnFileUnloading( const tResource& ownerResource )
	{
		for( u32 i = 0; i < mExportedFunctions.fCount( ); ++i )
			mExportedFunctions[ i ].mScriptObject.fDestroy( );
		for( u32 i = 0; i < mStandardExportedFunctions.fCount( ); ++i )
			mStandardExportedFunctions[ i ].mScriptObject.fDestroy( );
	}
	const Sqrat::Function& tScriptFile::fFindExportedFunction( const tStringPtr& exportName ) const
	{
		for( u32 i = 0; i < mExportedFunctions.fCount( ); ++i )
		{
			if( mExportedFunctions[ i ].mExportedName->fGetStringPtr( ) == exportName)
				return mExportedFunctions[ i ].mScriptObject.fTreatAsObject( );
		}

		return cNullScriptObj;
	}
}

