#include "ToolsPch.hpp"
#include "Momap.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tMotionMapFile.hpp"

namespace Sig { namespace Momap
{
	const char* fGetFileExtension( )
	{
		return ".momap";
	}

	b32 fIsMomapFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tConnection& o )
	{
		s( "i", o.mInput );
		s( "ii", o.mInputIndex );
		s( "o", o.mOutput );
		s( "oi", o.mOutputIndex );
	}

	//template<class tSerializer>
	//void fSerializeXmlObject( tSerializer& s, tContextOption& o )
	//{
	//	s( "n", o.mName );
	//	s( "k", o.mKey );
	//}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMoState& o )
	{
		s( "nextId", o.mNextUniqueNodeId );
		s( "c", o.mConnections );
		s( "n", o.mNodes );
	}


	void tMoState::fCollect( tDAGNodeCanvas* canvas, tDAGNodeCanvas::tDAGNodeList& nodes, tDAGNodeCanvas::tDAGNodeConnectionList& conn, b32 alreadyConstructed ) const
	{
		nodes.fSetCount( mNodes.fCount( ) );
		for( u32 i = 0; i < nodes.fCount( ); ++i )
		{
			nodes[ i ].fReset( mNodes[ i ].fGetRawPtr( ) );

			if( !alreadyConstructed )
			{
				tAnimBaseNode* aiNode = dynamic_cast<tAnimBaseNode*>( mNodes[ i ].fGetRawPtr( ) );
				aiNode->fApplyPropertyValues( );
				aiNode->fSetOwner( canvas );
			}
		}

		conn.fSetCount( mConnections.fCount( ) );
		for( u32 i = 0; i < conn.fCount( ); ++i )
		{
			const Momap::tConnection& connec = mConnections[ i ];
			if( !alreadyConstructed )
			{
				conn[ i ].fReset( new tDAGNodeConnection( 
					nodes[ connec.mInput ]->fInput( connec.mInputIndex ), 
					nodes[ connec.mOutput ]->fOutput( connec.mOutputIndex )
					) );
			}
			else
			{
				const tDAGNodeOutput::tConnectionList& list = nodes[ connec.mInput ]->fInput( connec.mInputIndex )->fConnectionList( );
				for( u32 l = 0; l < list.fCount( ); ++l )
				{
					if( &list[ l ]->fOutput( )->fOwner() == nodes[ connec.mOutput ] )
					{
						conn[ i ].fReset( list[ l ] );
						break;
					}
				}

				sigassert( conn[ i ] );
			}
		}

		//std::sort( nodes.fBegin( ), nodes.fEnd( ), fSortNodes );
	}




	//u32 tContext::fFindValueIndexByName( const std::string& name )
	//{
	//	return mOptions.fIndexOf( name );	
	//}

	//void tContext::fInsertValue( const std::string& name, u32 index )
	//{
	//	if( !mOptions.fFind( name ) )
	//	{
	//		mOptions.fPushBack( tContextOption( name, mNextFreeKey++ ) );
	//	}
	//}

	//void tContextData::fAddContext( const std::string& name )
	//{
	//	if( !mContexts.fFind( name ) )
	//	{
	//		mContexts.fPushBack( tContext( name, mNextFreeKey++ ) );
	//	}
	//}


	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s( "mostate", o.mMoState );
	}

	tFile::tFile( )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Momap", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Momap file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Momap", *this ) )
		{
			log_warning( "Couldn't load Momap file [" << path << "]" );
			return false;
		}

		return true;
	}

	namespace
	{
		void fBuildTree( tMotionMapFile::tTrack*& output, tAnimBaseNode& node, tMotionMapFile& fileOut )
		{
			output = new tMotionMapFile::tTrack( );

			tAnimBlendNode* bNode = dynamic_cast<tAnimBlendNode*>( &node );
			tAnimInputNode* aNode = dynamic_cast<tAnimInputNode*>( &node );
			if( aNode )
			{
				output->mAnim = new tMotionMapFile::tAnimTrackData( );
				output->mAnim->mName = fileOut.fAddLoadInPlaceStringPtr( aNode->fAnimName( ).c_str( ) );
				output->mAnim->mTimeScale = aNode->fTimeScale( );
				++fileOut.mNumAnimTracks;
			}
			else
			{
				tGrowableArray< tAnimBaseNode* > children;

				// Collect connected children
				for( u32 i = 0; i < node.fInputCount( ); ++i )
				{
					b32 empty = true;
					tDAGNodeInput* input = node.fInput( i ).fGetRawPtr( );
					for( u32 c = 0; c < input->fConnectionList( ).fCount( ); ++c )
					{
						tAnimBaseNode* bn = dynamic_cast<tAnimBaseNode*>( &(input->fConnectionList( )[ c ]->fOutput( )->fOwner( )) );
						if( bn )
						{
							empty = false;
							children.fPushBack( bn );
						}
						else
							log_warning( "Some non-tAnimBaseNode node detected in Momap file!" );
					}

					if( bNode && empty )
					{
						//for blend nodes, missing connections are important.
						children.fPushBack( NULL );
					}
				}

				output->mChildren.fResize( children.fCount( ) );

				if( bNode )
				{					
					f32 aCurve = bNode->fACurve( );
					f32 bCurve = bNode->fBCurve( );

					//if we have a blend node with only a connection on A, we need to swap it so there's only a connection on B.
					const b32 swapChildren = (children[ 1 ] == NULL);
					if( swapChildren )
					{
						fSwap( children[ 0 ], children[ 1 ] );
						fSwap( aCurve, bCurve );
					}

					++fileOut.mNumBlendTracks;
					output->mBlend = new tMotionMapFile::tBlendTrackData( 
						fileOut.fAddLoadInPlaceStringPtr( bNode->fBlendName( ).c_str( ) )
						, aCurve
						, bCurve
						, bNode->fTimeScale( )
						, bNode->fDigitalThreshold( )
						, bNode->fBehaviorDigital( )
						, bNode->fBehaviorOneShot( )
						);

					// these are backwards of the stack nodes for the time being
					for( s32 i = children.fCount( ) - 1; i >= 0; --i )
					{
						tMotionMapFile::tTrack* childResult = NULL;

						if( children[ i ] )
							fBuildTree( childResult, *children[ i ], fileOut );

						const u32 unReversedIndex = children.fCount( ) - i - 1;
						output->mChildren[ unReversedIndex ] = childResult;
					}
				}
				else
				{
					// stack node
					for( u32 i = 0; i < children.fCount( ); ++i )
					{
						tMotionMapFile::tTrack* childResult = NULL;

						if( children[ i ] )
							fBuildTree( childResult, *children[ i ], fileOut );

						output->mChildren[ i ] = childResult;
					}
				}
			}
		}
	}

 	tMotionMapFile* tFile::fBuildMoMapFile( b32 freshlyLoaded ) const
	{
		tDAGNodeCanvas::tDAGNodeConnectionList conns;
		tDAGNodeCanvas::tDAGNodeList nodes;

		mMoState.fCollect( NULL, nodes, conns, !freshlyLoaded );

		tAnimOutputNode* output = NULL;
		for( u32 i = 0; i < nodes.fCount( ); ++i )
		{
			output = dynamic_cast<tAnimOutputNode*>( nodes[ i ].fGetRawPtr( ) );
			if( output )
				break;
		}

		tMotionMapFile* result = NULL;

		if( output )
		{
			result = new tMotionMapFile( );		

			fBuildTree( result->mRoot, *output, *result );

			//result->mContexts.fResize( mMoState.mContexts.mContexts.fCount( ) );
			//for( u32 i = 0; i < result->mContexts.fCount( ); ++i )
			//	result->mContexts[ i ].mKey = mMoState.mContexts.mContexts[ i ].mKey;
				
			return result;			
		}
		else
			log_warning( "No output node found in Momap::tFile" );	

		return result;
	}

}}

