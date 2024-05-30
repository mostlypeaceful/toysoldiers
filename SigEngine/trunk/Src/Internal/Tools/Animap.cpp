#include "ToolsPch.hpp"
#include "Animap.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tAniMapFile.hpp"
#include "tAnimPackFile.hpp"
#include "tProjectFile.hpp"
#include "Editor/tEditablePropertyTypes.hpp"

namespace Sig { namespace Animap
{
	const char* fGetFileExtension( )
	{
		return ".animap";
	}

	b32 fIsAnimapFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile::tAnimRef& o )
	{
		s( "p", o.mAnipack );
		s( "a", o.mAnimName );
		s( "props", o.mProperties );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile::tContextSwitch& o )
	{
		s( "k", o.mContextKey );
		s( "b", o.mBranches );
		s( "l", o.mLeaves );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile::tMapping& o )
	{
		s( "n", o.mName );
		s( "a", o.mRoot );
	}

	namespace
	{
		static const char* cAnimTimeScalePropName = "Anim.TimeScale";
	}

	f32 tFile::tAnimRef::fTimeScale( ) const
	{
		return mProperties.fGetValue<f32>( cAnimTimeScalePropName, 1.f );
	}

	void tFile::tAnimRef::fAddCommonProps( )
	{
		mProperties.fInsert( tEditablePropertyPtr( NEW tEditablePropertyFloat( cAnimTimeScalePropName, 1.f, -999.f, 999.f, 0.1f, 2, false ) ) );
	}

	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s( "mappings", o.mMappings );
	}

	tFile::tFile( )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Animap", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Animap file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Animap", *this ) )
		{
			log_warning( "Couldn't load Animap file [" << path << "]" );
			return false;
		}

		return true;
	}

	namespace
	{
		void fBuildContextTree( tAniMapFile::tContextSwitch*& output, const tFile::tContextSwitch& input, tAniMapFile& file, tDynamicArray<u32>& contextsIndexedOut, const tProjectFile::tGameEnumeratedType* parentSwitch )
		{
			output->mBranches.fResize( input.mBranches.fCount( ) );
			for( u32 i = 0; i < output->mBranches.fCount( ); ++i )
			{
				const tFile::tContextSwitch& branchInput = input.mBranches[ i ];
				u32 contextIndex = ~0;
				const tProjectFile::tGameEnumeratedType* parentSwitchNextDepth = NULL;

				if( !parentSwitch )
				{
					// this index is the resultant type index in GameFlags.hpp
					parentSwitchNextDepth = tProjectFile::fInstance( ).fFindEnumeratedTypeByKey( branchInput.mContextKey );
					if( !parentSwitchNextDepth )
					{
						log_warning( "Couldn't find project enum key: " << branchInput.mContextKey );
						return; // bail :(
					}

					// keep track of the different contexts ever used
					u32 switchTypeValue = parentSwitchNextDepth->mKey;
					contextIndex = file.mContexts.fIndexOf( tAniMapFile::tContextRef( switchTypeValue ) );
					if( contextIndex == ~0 )
					{
						file.mContexts.fPushBack( tAniMapFile::tContextRef( switchTypeValue ) );
						contextIndex = file.mContexts.fCount( ) - 1;
					}

					// cache the context indexes we used on this mapping..
					contextsIndexedOut.fPushBack( contextIndex );
				}
				else 
				{
					contextIndex = parentSwitch->fFindValueIndexByKey( branchInput.mContextKey );
					if( contextIndex == ~0 )
					{
						log_warning( "Couldn't find project enum key: " << parentSwitch->mKey << " value key: " << branchInput.mContextKey );
						return; // bail :(
					}
				}

				tAniMapFile::tContextSwitch* result = new tAniMapFile::tContextSwitch( contextIndex );
				output->mBranches[ i ] = result;
				fBuildContextTree( result, branchInput, file, contextsIndexedOut, parentSwitchNextDepth );
			}

			output->mLeaves.fResize( input.mLeaves.fCount( ) );
			for( u32 i = 0; i < output->mLeaves.fCount( ); ++i )
			{
				output->mLeaves[ i ] = new tAniMapFile::tAnimRef(
					file.fAddLoadInPlaceResourcePtr( tResourceId::fMake<tAnimPackFile>( input.mLeaves[ i ].mAnipack.c_str( ) ) )
					, file.fAddLoadInPlaceStringPtr( input.mLeaves[ i ].mAnimName.c_str( ) )
					, input.mLeaves[ i ].fTimeScale( ) );
			}
		}
	}

	tAniMapFile* tFile::fMakeAnimapFile( ) const
	{
		tAniMapFile* result = new tAniMapFile( );

		result->mMappings.fResize( mMappings.fCount( ) );

		for( u32 m = 0; m < result->mMappings.fCount( ); ++m )
		{
			const tMapping& input = mMappings[ m ];
			tAniMapFile::tMapping& output = result->mMappings[ m ];

			output.mName = result->fAddLoadInPlaceStringPtr( input.mName.c_str( ) );
			output.mRoot = new tAniMapFile::tContextSwitch( ~0 );
			
			fBuildContextTree( output.mRoot, input.mRoot, *result, output.mContextsIndexed, NULL );			
		}

		return result;
	}

	namespace
	{
		void fGatherAnimPacksRecursive( tFilePathPtrList& output, tFile::tContextSwitch& root )
		{
			for( u32 i = 0; i < root.mBranches.fCount( ); ++i )
				fGatherAnimPacksRecursive( output, root.mBranches[ i ] );

			for( u32 i = 0; i < root.mLeaves.fCount( ); ++i )
				output.fFindOrAdd( tFilePathPtr( root.mLeaves[ i ].mAnipack.c_str( ) ) );
		}
	}

	void tFile::fGatherAnimPacks( tFilePathPtrList& output )
	{
		for( u32 i = 0; i < mMappings.fCount( ); ++i )
			fGatherAnimPacksRecursive( output, mMappings[ i ].mRoot );
	}

}}

