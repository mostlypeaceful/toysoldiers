#include "ToolsPch.hpp"
#include "Derml.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "FileSystem.hpp"

namespace Sig { namespace Derml
{
	const char* fGetFileExtension( )
	{
		return ".derml";
	}

	b32 fIsDermlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fDermlPathToMtlb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".derb" );
	}

	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tConnection& o )
	{
		s( "Input", o.mInput );
		s( "Index", o.mIndex );
		s( "Output", o.mOutput );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gDermlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gDermlVersion )
		{
			log_warning( "Derml file format is out of date -> Please re-export." );
			return;
		}

		s.fAsAttribute( "NextUniqueNodeId", o.mNextUniqueNodeId );
		s( "GeometryStyle", reinterpret_cast<int&>( o.mGeometryStyle ) );
		s( "Nodes", o.mNodes );
		s( "Connections", o.mConnections );
	}

	tFile::tFile( )
		: mVersion( gDermlVersion )
		, mNextUniqueNodeId( 0 )
		, mGeometryStyle( HlslGen::cVshMeshModel )
	{
	}
	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Derml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Derml file [" << path << "]" );
			return false;
		}

		return true;
	}
	void tFile::fSaveXml( tXmlSerializer & out )
	{
		out.fSave( "Derml", *this );
	}
	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Derml", *this ) )
		{
			log_warning( "Couldn't load Derml file [" << path << "]" );
			return false;
		}

		if( mVersion != gDermlVersion )
			return false;
		
		fFixupAfterLoad( );
		return true;
	}
	b32 tFile::fLoadXml( tXmlDeserializer & in )
	{
		if( !in.fLoad( "Derml", *this ) )
			return false;
		
		fFixupAfterLoad( );
		return true;
	}
	void tFile::fCollectOutputs( tShadeNode::tOutputArray& outputs ) const
	{
		for( u32 i = 0; i < outputs.fCount( ); ++i )
			outputs[ i ].fRelease( );
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			const tShadeNode::tOutputSemantic os = mNodes[ i ]->fOutputSemantic( );
			if( os < tShadeNode::cOutputCount )
			{
				sigassert( outputs[ os ].fNull( ) );
				outputs[ os ] = mNodes[ i ];
			}
		}
	}
	void tFile::fCullUnconnected( tNodeList& nodes ) const
	{
		// find relevant outputs
		tShadeNode::tOutputArray outputs;
		fCollectOutputs( outputs );

		// follow connections from outputs backward
		nodes.fSetCount( 0 );
		for( u32 i = 0; i < outputs.fCount( ); ++i )
		{
			if( outputs[ i ] )
			{
				nodes.fPushBack( outputs[ i ] );
				fCollectInputs( outputs[ i ], nodes );
			}
		}
	}
	namespace
	{
		struct tSortConnexByInput
		{
			inline b32 operator( )( const tConnection& a, const tConnection& b ) const
			{
				return a.mIndex < b.mIndex;
			}
		};
	}
	void tFile::fGetConnectionList( const tShadeNodePtr& shadeNode, tConnectionList& connexList ) const
	{
		const u32 nodeIndex = fPtrDiff( mNodes.fFind( shadeNode ), mNodes.fBegin( ) );

		// gather all connections being input to current node
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			if( mConnections[ i ].mInput == nodeIndex )
				connexList.fPushBack( mConnections[ i ] );

		// sort the incoming connections by input index
		std::sort( connexList.fBegin( ), connexList.fEnd( ), tSortConnexByInput( ) );
	}
	void tFile::fCollectInputs( tShadeNodePtr node, tNodeList& collected ) const
	{
		tConnectionList connex;
		fGetConnectionList( node, connex );

		// collect the nodes on the other side of the connection
		for( u32 i = 0; i < connex.fCount( ); ++i )
		{
			tShadeNodePtr toAdd = mNodes[ connex[ i ].mOutput ];
			if( !collected.fFind( toAdd ) )
			{
				// add the node
				collected.fPushBack( toAdd );

				// recurse on the node we just added
				fCollectInputs( toAdd, collected );
			}
		}
	}
	void tFile::fFixupAfterLoad( )
	{
		// Ensure our shade nodes have unique ids
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			tShadeNodePtr & node = mNodes[ i ];
			for( u32 j = 0; j < mNodes.fCount( ); ++j )
			{
				// Don't test against ourselves
				if( i == j ) 
					continue;

				tShadeNodePtr & testNode = mNodes[ j ];

				// Not the same id
				if( node->fUniqueNodeId( ) != testNode->fUniqueNodeId( ) )
					continue;

				// Conflict - must resolve
								
				// If we're editable or the other isn't then update the other
				if( node->fMatEdAllowEdit( ) || !testNode->fMatEdAllowEdit( ) )
				{
					if( testNode->fMatEdAllowEdit( ) )
						log_line( 0, "ATTENTION: Irresolvable material node conflict: " << testNode->fMatEdName( ) << " edits may have been lost" );

					testNode->fSetUniqueNodeId( mNextUniqueNodeId++ );
				}

				// Otherwise we're not editable and the other is so update us
				else
				{
					node->fSetUniqueNodeId( mNextUniqueNodeId++ );
					break; // No more to process for this node
				}
			}
		}
	}


	///
	/// \section tMtlFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMtlFile& o )
	{
		s( "ShaderPath", o.mShaderPath );
		s( "Nodes", o.mNodes );
		s( "ShaderLastModifiedStamp", o.mShaderLastModifiedStamp );
	}

	tMtlFile::tMtlFile( )
		: mShaderLastModifiedStamp( 0 )
	{
	}

	b32 tMtlFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Derml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Derml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tMtlFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Derml", *this ) )
		{
			log_warning( "Couldn't load Derml file [" << path << "]" );
			return false;
		}

		return true;
	}

	void tMtlFile::fSerialize( tXmlSerializer& ser )
	{
		fSerializeXmlObject( ser, *this );
	}

	void tMtlFile::fSerialize( tXmlDeserializer& des )
	{
		fSerializeXmlObject( des, *this );
	}

	void tMtlFile::fClone( tMtlFile& cloneOut, const tFilePathPtr& scratchFilePath )
	{
		fSaveXml( scratchFilePath, false );
		cloneOut.fLoadXml( scratchFilePath );
	}

	void tMtlFile::fFromShaderFile( const tFile& dermlFile, const tFilePathPtr& shaderFilePath )
	{
		mShaderPath = shaderFilePath;
		mShaderLastModifiedStamp = fShaderLastModified( );
		dermlFile.fCullUnconnected( mNodes );
	}

	b32 tMtlFile::fUpdateAgainstShaderFile( Derml::tFile& dermlFile, b32 * loadSuccess )
	{
		if( mShaderPath.fNull( ) )
			return false;

		if( !fIsShaderModified( ) )
			return false; // shader is up to date

		const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( mShaderPath );

		dermlFile = Derml::tFile( );
		if( dermlFile.fLoadXml( absolutePath ) )
		{
			if( loadSuccess )
				*loadSuccess = true;

			// save off current nodes
			tMtlFile me = *this;

			// reset from shader
			fFromShaderFile( dermlFile, mShaderPath );

			// Restore values from all nodes that exist in 'me'
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				tShadeNodePtr dest = mNodes[ i ];
				tShadeNodePtr src = me.fFindByUniqueId( dest->fUniqueNodeId( ) );

				if( src )
					dest->fRefreshMaterialProperties( *src, i );
			}
		}
		else
		{
			if( loadSuccess )
				*loadSuccess = false;

			// reset
			*this = tMtlFile( );
		}

		return true;
	}

	b32 tMtlFile::fUpdateAgainstPreviousNodes( const tNodeList& oldNodes )
	{
		b32 modified = false;

		for( u32 i = 0; i < oldNodes.fCount( ); ++i )
		{
			tShadeNodePtr src = oldNodes[ i ];
			const u32 dstIndex = fFindByName( src->fMatEdName( ).c_str( ) );
			if( dstIndex < mNodes.fCount( ) )
			{
				mNodes[ dstIndex ]->fRefreshMaterialProperties( *src, dstIndex );
				modified = true;
			}
		}

		return modified;
	}

	b32 tMtlFile::fIsShaderModified( u64* newStampOut ) const
	{
		if( mShaderPath.fNull( ) )
			return false;
		const u64 newTimeStamp = fShaderLastModified( );
		if( newStampOut ) *newStampOut = newTimeStamp;
		return( newTimeStamp != mShaderLastModifiedStamp );
	}

	u64 tMtlFile::fShaderLastModified( ) const
	{
		const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( mShaderPath );
		const u64 newTimeStamp = FileSystem::fGetLastModifiedTimeStamp( absolutePath );
		return newTimeStamp;
	}

	tShadeNodePtr tMtlFile::fFindByUniqueId( u32 uid ) const
	{
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			if( mNodes[ i ]->fUniqueNodeId( ) == uid )
				return mNodes[ i ];
		}

		return tShadeNodePtr( );
	}

	u32 tMtlFile::fFindByName( const char* name ) const
	{
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			if( _stricmp( mNodes[ i ]->fMatEdName( ).c_str( ), name ) == 0 )
				return i;
		}

		return ~0;
	}

}}

