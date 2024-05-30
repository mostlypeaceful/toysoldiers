#include "ToolsPch.hpp"
#include "Goaml.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "FileSystem.hpp"
#include "tScriptFileConverter.hpp"
#include "AI/tSigAIGoal.hpp"
#include "tProjectFile.hpp"

namespace Sig { namespace Goaml
{
	const char* fGetFileExtension( )
	{
		return ".goaml";
	}

	b32 fIsGoamlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fGoamlPathToNutb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".nutb" );
	}
	tFilePathPtr fGoamlPathToNut( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".nut" );
	}

	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tConnection& o )
	{
		s( "Input", o.mInput );
		s( "InputIndex", o.mInputIndex );
		s( "Output", o.mOutput );
		s( "OutputIndex", o.mOutputIndex );
		s( "Props", o.mProps );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gGoamlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gGoamlVersion )
		{
			log_warning( "Goaml file format is out of date -> Please re-export." );
			return;
		}

		s.fAsAttribute( "NextUniqueNodeId", o.mNextUniqueNodeId );
		s( "Nodes", o.mNodes );
		s( "Connections", o.mConnections );
		s( "MoMapFile", o.mMoMapFile ); 
		s( "Depends", o.mExplicitDependencies ); 
	}

	tFile::tFile( )
		: mVersion( gGoamlVersion )
		, mNextUniqueNodeId( 0 )
	{
	}
	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Goaml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Goaml file [" << path << "]" );
			return false;
		}

		return true;
	}
	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Goaml", *this ) )
		{
			log_warning( "Couldn't load Goaml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 fSortNodes( const tDAGNodePtr& aa, const tDAGNodePtr& bb )
	{
		tGoalAINode* a = dynamic_cast<tGoalAINode*>( aa.fGetRawPtr( ) );
		tGoalAINode* b = dynamic_cast<tGoalAINode*>( bb.fGetRawPtr( ) );

		b32 aIsBase = a->fOutputClassName( ).length( ) > 0; //a->fOutputClassName( ) == b->fBaseClass( ) && a->fOutputClassName( ).length( ) > 0 && b->fBaseClass( ).length( ) > 0;
		b32 bIsBase = b->fOutputClassName( ).length( ) > 0; //b->fOutputClassName( ) == a->fBaseClass( ) && b->fOutputClassName( ).length( ) > 0 && a->fBaseClass( ).length( ) > 0;

		if( aIsBase != bIsBase ) 
			return aIsBase;
		else 
			return a < b;
	}

	void tFile::fQuickSortXMLNodes( )
	{
		// Check if already sorted
		for( u32 i = 0; i < mNodes.fCount() - 1; ++i )
		{
			if( mNodes[ i ]->fUniqueNodeId() > mNodes[ i ]->fUniqueNodeId() )
			{
				// Quick sort
				fQuickSortXMLNodes( 0, mNodes.fCount() - 1 );
				break;
			}
		}
	}

	// Standard quick sort
	void tFile::fQuickSortXMLNodes( s32 left, s32 right )
	{
        u32 pivot = mNodes[(left + right) / 2]->fUniqueNodeId( );
        s32 i = left, j = right;

        while( i <= j )
		{
			while ( mNodes[ i ]->fUniqueNodeId( ) < pivot )
				++i;
			while( mNodes[ j ]->fUniqueNodeId( ) > pivot )
				--j;
			if( i <= j )
			{
                //swap
                tAINodePtr tmp = mNodes[ i ];
                mNodes[ i ] = mNodes[ j ];
                mNodes[ j ] = tmp;

                // Keep the connection indexes in the proper order
                for ( u32 m = 0; m < mConnections.fCount( ); ++m )
                {
                    Goaml::tConnection connection = mConnections[ m ];

                    if ( connection.mInput == i )
                        connection.mInput = j;
                    else if ( connection.mInput == ( j ) )
                        connection.mInput = i;

                    if ( connection.mOutput == i )
                        connection.mOutput = j;
                    else if ( connection.mOutput == ( j ) )
                        connection.mOutput = i;

                    mConnections[ m ] = connection;
                }

                ++i;
                --j;
            }
        }

        if( left < j )
            fQuickSortXMLNodes( left, j );
        if( i < right )
            fQuickSortXMLNodes( i, right );
    }

	void tFile::fSortXMLNodes( )
	{
		fQuickSortXMLNodes( );
	}

	void tFile::fCollect( tDAGNodeCanvas* canvas, tDAGNodeCanvas::tDAGNodeList& nodes, tDAGNodeCanvas::tDAGNodeConnectionList& conn, b32 alreadyConstructed ) const
	{
		nodes.fSetCount( mNodes.fCount( ) );
		for( u32 i = 0; i < nodes.fCount( ); ++i )
		{
			nodes[ i ].fReset( mNodes[ i ].fGetRawPtr( ) );

			if( !alreadyConstructed )
			{
				tAINode* aiNode = dynamic_cast<tAINode*>( mNodes[ i ].fGetRawPtr( ) );
				aiNode->fApplyPropertyValues( );
				aiNode->fSetOwner( canvas );
			}
		}

		conn.fSetCount( mConnections.fCount( ) );
		for( u32 i = 0; i < conn.fCount( ); ++i )
		{
			const Goaml::tConnection& connec = mConnections[ i ];
			if( !alreadyConstructed )
			{
				conn[ i ].fReset( new tDAGNodeConnection( 
					nodes[ connec.mInput ]->fInput( connec.mInputIndex ), 
					nodes[ connec.mOutput ]->fOutput( connec.mOutputIndex )
					) );

				tAIConnectionData* data = new tAIConnectionData( connec.mProps );
				conn[ i ]->fSetData( tDAGDataPtr( data ) );
				data->fSetConnection( conn[ i ] );
				data->fReadProps( );
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

		std::sort( nodes.fBegin( ), nodes.fEnd( ), fSortNodes );

		if( !alreadyConstructed )
		{
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				if( tGoalAINode* aiNode = dynamic_cast<tGoalAINode*>( mNodes[ i ].fGetRawPtr( ) ) )
					aiNode->fFixSystemHandlers( conn );
			}
		}
	}


	// conversion to .nut
	namespace
	{
		struct tGoamlData
		{
			std::string mUniqueFileTag;
			tDAGNodeCanvas::tDAGNodeList mNodes;
			tDAGNodeCanvas::tDAGNodeConnectionList mConns;

			//temp data for class construction
			std::string mExtraFunctions; 
			u32 mNumberOfNameOverriddenClasses;

			tGoamlData( )
				: mNumberOfNameOverriddenClasses( 0 )
			{ }

			tDAGNodeCanvas::tDAGNodeConnectionList fCollectConnections( const tGoalEventHandlerPtr& handler ) const
			{
				tDAGNodeCanvas::tDAGNodeConnectionList result;
				if( !handler ) return result;

				const tDAGNodeOutput::tConnectionList& list = handler->mOutput->fConnectionList( );
				for( u32 i = 0; i < list.fCount( ); ++i )
					result.fPushBack( tDAGNodeConnectionPtr( list[ i ] ) );

				return result;
			}
		};

		const tGoalAINode& fGoalAINode( const tDAGNodePtr& p )
		{
			const tGoalAINode* goalNode = dynamic_cast<tGoalAINode*>( p.fGetRawPtr( ) );
			sigassert( goalNode );
			return *goalNode;
		}

		void fString( std::string& out, const std::string& string )
		{ 
			out += string;
		}

		void fTab( std::string& out, u32 count )
		{ 
			for( u32 i = 0; i < count; ++i )
				out += "\t";
		}

		std::string fClassName( const tGoalAINode& node, const tGoamlData& data, b32* overRidden = NULL )
		{
			std::string nameOverride = node.fOutputClassName( );
			if( nameOverride.length( ) > 0 )
			{
				if( overRidden ) *overRidden = true;
				return nameOverride;
			}
			else
			{
				// generate safe name
				if( overRidden ) *overRidden = false;
				u32 index = data.mNodes.fIndexOf( &node );
				return data.mUniqueFileTag + StringUtil::fToString( index );
			}
		}

		std::string fTerminateForConnection( const tGoalAINode & dest, const tGoalAINode & src, const tGoamlData & data, b32 markAsComplete  )
		{
			if( &dest == &src )
			{
				if( markAsComplete )
					return "MarkAsComplete( )\n";
				else 
					return "Terminate( )\n";
			}
			else
			{
				if( markAsComplete )
					return "CompleteGoalsNamed( \"" + fClassName( dest, data ) + "\" )\n";
				else
					return "TerminateGoalsNamed( \"" + fClassName( dest, data ) + "\" )\n";
			}
		}

		std::string fFunctionName( const tGoalAINode& node, const tGoamlData& data, const std::string& base )
		{
			// generate safe name
			u32 index = data.mNodes.fIndexOf( &node );
			return base + data.mUniqueFileTag + StringUtil::fToString( index );
		}

		struct tFire
		{
			std::string mWhiteSpace;
			std::string mOGText;		// the entirety of the original fire command
			std::string mParameterText; // the contents of the param table arg
			u32 mStartIndex;

			tFire( ) : mStartIndex( 0 ) { }
		};

		u32 fFindFires( const std::string& text, tGrowableArray< tFire >& fires )
		{
			tGrowableArray< std::string > lines;
			u32 startIndex = 0;
			StringUtil::fSplit( lines, text.c_str( ), "\n", true );

			const std::string cFire = "fire";
			const u32 cFireLen = cFire.length( );

			for( u32 i = 0; i < lines.fCount( ); ++i )
			{
				const char* lineStart = lines[ i ].c_str( );
				const char* start = lineStart;

				do 
				{
					start = StringUtil::fStrStrI( start, cFire.c_str( ) );
					if( start )
					{
						//found a fire, make sure it's actually a fire command, by checking previous character
						const char* last = lineStart + lines[ i ].length( );
						const char* prev = start - 1;
						const char* next = fMin( start + 4, last );
						if( (prev < lineStart || *prev == ' ' || *prev == '\t' || *prev == '/' || *prev == '{' || *prev == ')')
							&& (next >= last || *next == '(' || *next == ' ' || *next == '\t' || *next == '/') ) // '/' means after a /*comment block*/fire( params )
						{
							const u32 lineOffset = start - lineStart;
							fires.fPushBack( tFire( ) );
							tFire& newFire = fires.fBack( );
							newFire.mStartIndex = startIndex + lineOffset;
							newFire.mWhiteSpace = StringUtil::fReplaceNonWhiteSpaceWithWhiteSpace( std::string( start - lineOffset, start ) );

							const char *paramStart = StringUtil::fReadUntilNewLineOrCharacter( start, '(' );
							const char *end = StringUtil::fReadUntilNewLineOrCharacter( start, ')' );
							if( *end == ')' ) ++end;

							// -1 and +1 remove the ) and (
							s32 paramLen = end - 1 - paramStart + 1;
							if( paramLen > 0 )
							{
								newFire.mParameterText = StringUtil::fEatWhiteSpace( std::string( paramStart + 1, end - 1 ) );
							}
							else
							{
								newFire.mParameterText = "params";
							}

							newFire.mOGText = std::string( start, end );						
						}

						// skip over this fire and keep looking
						start = next;
					}
				}
				while( start );

				// plus 1 for the newline
				startIndex += lines[ i ].length( ) + 1;
			}

			return fires.fCount( );
		}

		std::string fConstructClass( const tGoalAINode* destination, const tGoamlData& data, const tFire& fire )
		{
			return fClassName( *destination, data ) + "( " + fire.mParameterText + " )";
		}

		std::string fConvertScript( tGoamlData& data, const tGoalEventHandlerPtr& handler, b32 forPolled = false, b32 forSwitch = false, b32 forSequence = false );
		void fWriteSnippet( std::string& out, u32 indent, const std::string& snippet );

		std::string fBuildConnection( tGoamlData& data, tDAGNodeConnectionPtr& conn, const tFire& fire, b32 forPolled, b32 forSwitch )
		{
			tAIConnectionData* connData = dynamic_cast< tAIConnectionData* >( conn->fData( ).fGetRawPtr( ) );
			sigassert( connData );

			std::string result;

			tAIConnectionData::tBehavior behavior = connData->fBehavior( );
			u32 maxClear = connData->fMaxClearablePriority( );

			const tGoalAINode* destination = dynamic_cast< const tGoalAINode* >( &conn->fInput( )->fOwner( ) );
			sigassert( destination );

			u32 index = conn->fInput( )->fIndex( );
			if( index == tGoalAINode::cInputTerminate || index == tGoalAINode::cInputSuspend )
			{
				const tGoalAINode* origin = dynamic_cast< const tGoalAINode* >( &conn->fOutput( )->fOwner( ) );
				sigassert( origin );

				result = fTerminateForConnection( *destination, *origin, data, index == tGoalAINode::cInputTerminate );
			}
			else if( destination->fType( ) == tGoalAINode::cSwitchType )
			{
				std::string functionName = fFunctionName( *destination, data, "Switch" );
                result = "if( " + functionName + "( " + fire.mParameterText + " ) )\n" + fire.mWhiteSpace + "{ \n" + fire.mWhiteSpace + "\treturn true \n" + fire.mWhiteSpace + "}\n";

				//build the actual function to call
                std::string newFunction = "function " + functionName + "( params )\n{\n";
				for( u32 i = 0; i < destination->fUserHandlerCount( ); ++i )
				{
					std::string script = fConvertScript( data, tGoalEventHandlerPtr( destination->fUserHandler( i ) ), false, true );
					fWriteSnippet( newFunction, fire.mWhiteSpace.length( ), script );
				}

                newFunction += "\treturn false\n";
				data.mExtraFunctions += newFunction + "}\n";
			}
			else if( destination->fType( ) == tGoalAINode::cSequenceType )
			{
				std::string newFunction = fFunctionName( *destination, data, "Sequence" );
				result = newFunction + "( " + fire.mParameterText + " )\n";

				//build the actual function to call
				newFunction = "function " + newFunction + "( params )\n{\n";

				// push these in reverse order to fill stack
				for( s32 i = destination->fUserHandlerCount( ) - 1; i >= 0 ; --i )
				{
					std::string script = fConvertScript( data, tGoalEventHandlerPtr( destination->fUserHandler( i ) ), false, false, true );
					fWriteSnippet( newFunction, 1, script );
				}

				data.mExtraFunctions += newFunction + "}\n";
			}
			else
			{
				// pushing a goal
				std::string destComment = destination->fDisplayableName( );

				b32 check = (behavior != tAIConnectionData::cBehaviorSwitch && behavior != tAIConnectionData::cBehaviorPushBelow);
				std::string ifStatementPad;

				if( check )
				{
					ifStatementPad = "\t";
					result = "if( ChildPriority( ) <= " + StringUtil::fToString( connData->fMaxClearablePriority( ) ) + " )\n" + fire.mWhiteSpace + "{\n" + fire.mWhiteSpace;
				}

				if( destination->fSingleInstance( ) )
				{
					const tGoalAINode* origin = dynamic_cast< const tGoalAINode* >( &conn->fOutput( )->fOwner( ) );
					sigassert( origin );

					result += ifStatementPad + fTerminateForConnection( *destination, *origin, data, false ) + fire.mWhiteSpace;
				}

				switch( behavior )
				{
				case tAIConnectionData::cBehaviorSwitch:
					result += ifStatementPad + "SwitchToGoal( /* " + destComment + " */\n\t" + fire.mWhiteSpace + ifStatementPad + fConstructClass( destination, data, fire ) + " )";
					break;
				case tAIConnectionData::cBehaviorClearAndPush:
					result += ifStatementPad + "ClearAndPushGoal( /* " + destComment + ", UpTo:*/ " + StringUtil::fToString( maxClear ) + ",\n\t" + fire.mWhiteSpace + ifStatementPad + fConstructClass( destination, data, fire ) + " )";
					break;
				case tAIConnectionData::cBehaviorPush:
					result += ifStatementPad + "PushGoal( /* " + destComment + " */\n\t" + fire.mWhiteSpace + ifStatementPad + fConstructClass( destination, data, fire ) + " )";
					break;
				case tAIConnectionData::cBehaviorPushBelow:
					result += ifStatementPad + "PushBelow( /* " + destComment + ", Pri:*/ " + StringUtil::fToString( maxClear ) + ",\n\t" + fire.mWhiteSpace + ifStatementPad + fConstructClass( destination, data, fire ) + " )";
					break;
				}

				if( forPolled ) result += "\n" + fire.mWhiteSpace + ifStatementPad + "return true";

				if( check ) result += "\n" + fire.mWhiteSpace + "}\n";
				else result += "\n";
			}

			if( forSwitch ) result += fire.mWhiteSpace + "return false\n";

			return result;
		}

		std::string fConvertScript( tGoamlData& data, const tGoalEventHandlerPtr& handler, b32 forPolled, b32 forSwitch, b32 forSequence )
		{
			std::string result;

			if( handler ) 
			{
				tDAGNodeCanvas::tDAGNodeConnectionList potentialConns = data.fCollectConnections( handler );
				if( handler->fScriptValid( ) || potentialConns.fCount( ) )
				{
					b32 aiFlagEvent = (forSwitch || forSequence) && (handler->fType( ) == tGoalEventHandler::cAIFlagEvent);
					if( aiFlagEvent )
					{
						std::string notIzer = handler->mAIFlagGoesTrue ? "" : "!";
						result += "if( " + notIzer + "Logic.AIData.Value( " + handler->fAIFlagIndex( ) + " /*" + handler->fName( ) + "*/ ) )\n{\n";
					}

					fWriteSnippet( result, aiFlagEvent ? 1 : 0, handler->fScript( ) );

					tGrowableArray< tFire > fires;
					u32 fireCount = fFindFires( result, fires );

					if( potentialConns.fCount( ) > 0 && fires.fCount( ) == 0 )
					{
						//no fire found, append a fire all to the end.
						fires.fPushBack( tFire( ) );
						tFire& newFire = fires.fBack( );

						newFire.mOGText = "fire";
						newFire.mParameterText = "params";

						// insert before last occurance of return..
						const std::string cReturn = "return";

						u32 insertPt = result.rfind( cReturn );
						if( insertPt == std::string::npos )
							insertPt = result.length( );

						newFire.mStartIndex = insertPt + 1; //for the new line
						result.insert( insertPt, "\n" + newFire.mOGText );
					}

					// replace fires
					s32 lenChanged = 0;

					for( u32 f = 0; f < fires.fCount( ); ++f )
					{
						tFire& fire = fires[ f ];

						// Brace up the fires incase the user forgot to, there may be multiple, plus potentially a return.
						std::string oldWhiteSpace = fire.mWhiteSpace;
						fire.mWhiteSpace += "\t";
						std::string newString;

						if( aiFlagEvent )
						{
							oldWhiteSpace += "\t";
							newString += "\t";
							fire.mWhiteSpace += "\t";
						}

						newString += "{\n" + fire.mWhiteSpace;

						{
							sigassert( potentialConns.fCount( ) <= 1 );

							for( u32 c = 0; c < potentialConns.fCount( ); ++c )
							{
								const tAIConnectionData* connData = dynamic_cast< const tAIConnectionData* >( potentialConns[ c ]->fData( ).fGetRawPtr( ) );
								newString += fBuildConnection( data, potentialConns[ c ], fire, forPolled, forSwitch );
								if( c < potentialConns.fCount( ) - 1 ) newString += "\n";
							}
						}

						newString += oldWhiteSpace + "}\n";

						const char* curStart = result.c_str( );
						const char* curBegin = curStart + fire.mStartIndex + lenChanged;
						const char* curEnd = curBegin + fire.mOGText.length( );
						const char* end = result.c_str( ) + result.length( );
						result = std::string( curStart, curBegin ) + newString + std::string( curEnd, end );

						s32 diff = newString.length( ) - fire.mOGText.length( );
						lenChanged += diff;
					}

					if( aiFlagEvent )
					{
						result += "}\n";
					}
				}
			}

			return result;
		}

		void fWriteSnippet( std::string& out, u32 indent, const std::string& snippet )
		{
			tGrowableArray< std::string > lines;
			StringUtil::fSplit( lines, snippet.c_str( ), "\n" );

			for( u32 i = 0; i < lines.fCount( ); ++i )
			{
				const std::string& line = lines[ i ];
				if( line.length( ) > 0 )
				{
					fTab( out, indent );
					fString( out, line + "\n" );
				}
			}
		}

		void fClass( std::string& out, tGoamlData& data, u32 index )
		{
			const tGoalAINode& node = fGoalAINode( data.mNodes[ index ] );
			if( node.fType( ) != tGoalAINode::cGoalType ) return;

			// collect handlers
			tGoalEventHandlerPtr onActivate;
			tGoalEventHandlerPtr onSuspend;
			tGoalEventHandlerPtr onComplete;
			tGoalEventHandlerPtr onTick;
			tGoalEventHandlerPtr potential;
			tGoalEventHandlerPtr firstChance;
			tGrowableArray< std::pair< tGoalEventHandlerPtr, std::string > > events;
			tGrowableArray< std::pair< tGoalEventHandlerPtr, std::string > > polled;
			tGrowableArray< std::pair< tGoalEventHandlerPtr, std::string > > aiFlagChecks;

			for( u32 i = 0; i < node.fHandlerCount( ); ++ i )
			{
				tGoalEventHandlerPtr handler( node.fHandler( i ) );

				if( handler->fType( ) == tGoalEventHandler::cOnActivateType )		onActivate = handler;
				else if( handler->fType( ) == tGoalEventHandler::cOnSuspendType )	onSuspend = handler;
				else if( handler->fType( ) == tGoalEventHandler::cOnCompleteType )	onComplete = handler;
				else if( handler->fType( ) == tGoalEventHandler::cOnTickType )		onTick = handler;
				else if( handler->fType( ) == tGoalEventHandler::cPotentialType )	potential = handler;
				else if( handler->fType( ) == tGoalEventHandler::cFirstChanceType ) firstChance = handler;
				else if( handler->fType( ) == tGoalEventHandler::cAIFlagEvent )
				{
					std::string script = fConvertScript( data, handler );
					if( script.length( ) > 0 )
						aiFlagChecks.fPushBack( std::pair< tGoalEventHandlerPtr, std::string >( handler, script ) );
				}
				else if( handler->fType( ) == tGoalEventHandler::cRegularType ) 
				{
					std::string script = fConvertScript( data, handler );
					if( script.length( ) > 0 )
						events.fPushBack( std::pair< tGoalEventHandlerPtr, std::string >( handler, script ) );
				}
				else if( handler->fType( ) == tGoalEventHandler::cPolledType ) 
				{
					std::string script = fConvertScript( data, handler, true );
					if( script.length( ) > 0 )
						polled.fPushBack( std::pair< tGoalEventHandlerPtr, std::string >( handler, script ) );
				}
			}

			b32 overRiddenName = false;
			const std::string className = fClassName( node, data, &overRiddenName );
			std::string baseClass = node.fBaseClass( );
			if( baseClass.length( ) == 0 ) baseClass = "AI.SigAIGoal";
			const std::string moState = node.fMotionState( );
			const b32 oneShot = node.fOneShot( );
			const b32 persist = node.fPersist( );

			// Write class
			fString( out, "/////////////////////////////////////////////\n" );
			fString( out, "//// ------ " + node.fVerboseName( ) + " ------\n" );
			fString( out, "/////////////////////////////////////////////\n" );
			fString( out, "class " + className + " extends " + baseClass + "\n" );
			fString( out, "{\n" );		
			fString( out, "\tparams = 0 //user data\n\tparams_ = 0 //construction data\n" );

			//if( overRiddenName )
			//{
			//	if( ++data.mNumberOfNameOverriddenClasses > 1 )
			//		log_warning( "More than one class with an overridden output class name found in the goaml.\n Overridden class names are assumed to be top level and will return true for all event handlers." );
			//}


			// Constructor
			fString( out, "\tconstructor( paramsIn )\n\t{\n\t\tparams = paramsIn\n\t\tparams_ = paramsIn\n\t\t" + baseClass + ".constructor( params )\n" );
			fString( out, "\t\tSetup( " + StringUtil::fToString( node.fPriority( ) ) + ", \"" + className + "\", this )\n" );
			if( persist )  
				fString( out, "\n\t\tPersist( )\n" );

			fString( out, "\t}\n" );

			// first chance
			fString( out, "\tfunction OnInsertion( goalDriven )\n\t{\n" );
			fString( out, "\t\t" + baseClass + ".OnInsertion( goalDriven )\n" );
			// These modes are exclusive and "references" takes presidency if available.
			if( node.mMoStateReferences.fCount( ) )
			{
				for( u32 i = 0; i < node.mMoStateReferences.fCount( ); ++i )
					fString( out, "\t\tAddMoStateRef( \"" + node.mMoStateReferences[ i ].mName + "\" )\n" );
			}
			else if( moState.length( ) > 0 ) 
				fString( out, "\t\tSetMotionState( " + moState + ", params, " + (oneShot ? "true" : "false") + " )\n" );

			std::string firstChanceScript = fConvertScript( data, firstChance );
			if( firstChanceScript.length( ) > 0 )
				fWriteSnippet( out, 2, firstChanceScript );
			fString( out, "\t}\n" );

			// Debug Name
			fString( out, "\tfunction DebugTypeName( )\n\t\treturn \"" + node.fVerboseName( ) + "\" \n" );

			// Activate
			std::string activateScript = fConvertScript( data, onActivate );
			std::string oneShotScript = node.fOneShotOverrideScript( );
			if( activateScript.length( ) > 0 || oneShotScript.length( ) > 0 )
			{
				fString( out, "\n\tfunction OnActivate( )\n\t{\n" );
				if( activateScript.length( ) > 0 ) fWriteSnippet( out, 2, activateScript );
				fString( out, "\t\t" + baseClass + ".OnActivate( )\n" ); //run our on activate stuff before children get activated
				if( oneShotScript.length( ) > 0 ) fString( out, "\t\tSetOneShotTimer( " + oneShotScript + " )\n\n" ); //run this last as it will override the timer set in onActivate (potentially)
				fString( out, "\t}\n" );
			}

			// Suspend
			std::string completeScript = fConvertScript( data, onComplete );
			std::string suspendScript = fConvertScript( data, onSuspend );
			b32 hasComplete = completeScript.length( ) > 0;
			b32 hasSuspend = suspendScript.length( ) > 0;
			if( hasSuspend || hasComplete )
			{
				fString( out, "\n\tfunction OnSuspend( )\n\t{\n" );
				fString( out, "\t\t" + baseClass + ".OnSuspend( )\n\n" ); //run children suspend before our suspend stuff

				if( hasSuspend )
				{
					fWriteSnippet( out, 2, suspendScript );
					if( hasComplete ) fString( out, "\n" );
				}

				if( hasComplete )
				{
					fString( out, "\t\tif( ExitMode == SIGAIGOAL_EXIT_COMPLETED )\n\t\t{\n" );
					fWriteSnippet( out, 3, completeScript );
					fString( out, "\t\t}\n" );
				}

				fString( out, "\t}\n" );
			}
			
			// Tick
			std::string tickScript = fConvertScript( data, onTick );
			if( tickScript.length( ) > 0 )
			{
				fString( out, "\n\tfunction OnTick( )\n\t{\n" );

				fWriteSnippet( out, 2, tickScript );

				fString( out, "\t}\n" );
			}

			// Potential
			std::string potentialScript = fConvertScript( data, potential );
			if( potentialScript.length( ) > 0 )
			{
				fString( out, "\n\tfunction " + std::string( AI::tSigAIGoal::cSigAIPotentialFunction ) + "( )\n\t{\n" );	
				fWriteSnippet( out, 2, potentialScript );
				fString( out, "\t}\n" );
			}

			//// Polled
			//if( polled.fCount( ) > 0 )
			//{
			//	fString( out, "\n\tfunction " + std::string( AI::tSigAIGoal::cSigAIPolledFunction ) + "( )\n\t{\n" );	
			//	for( u32 i = 0; i < polled.fCount( ); ++i )
			//		fWriteSnippet( out, 2, polled[ i ].second );
			//	fString( out, "\t\treturn false\n\t}\n" );
			//}

			// Event Handler
			if( events.fCount( ) > 0 || aiFlagChecks.fCount( ) > 0 )
			{
				fString( out, "\n\tfunction HandleLogicEvent( event )\n\t{\n" );
				fString( out, "\t\tif( " + baseClass + ".HandleLogicEvent( event ) )\n\t\t\treturn true\n\n" );
				
				if( events.fCount( ) > 0 )
				{
					fString( out, "\t\tswitch( event.Id )\n\t\t{\n" );
					for( u32 i = 0; i < events.fCount( ); ++i )
					{
						const tGoalEventHandlerPtr& handler = events[ i ].first;
						const std::string& script = events[ i ].second;

						fString( out, "\t\t\tcase " + tProjectFile::tEvent::fCppName( handler->fName( ) ) + ":\n\t\t\t{\n" );
						fWriteSnippet( out, 4, script );

						//std::string closer = overRiddenName ? "return true" : "break"; //if this appears to be a top level class, indicate events were handled
						fString( out, "\t\t\t}\n\t\t\tbreak\n" );
					}
					fString( out, "\t\t}\n" );
				}

				fString( out, "\n" );

				if( aiFlagChecks.fCount( ) > 0 )
				{
					const tProjectFile& projectFile = tProjectFile::fInstance( );
					std::string elseText = "";

					for( u32 i = 0; i < aiFlagChecks.fCount( ); ++i )
					{
						const tGoalEventHandlerPtr& handler = aiFlagChecks[ i ].first;
						const std::string& script = aiFlagChecks[ i ].second;
						
						fString( out, "\t\t" + elseText + "if( event.Id == " + handler->fAIFlagCheckEventID( ) + " /*" + handler->fName( ) + "*/ )\n\t\t{\n" );
						fWriteSnippet( out, 3, script );

						fString( out, "\t\t}\n" );
						elseText = "else ";
					}
				}

				fString( out, "\n\t\treturn false\n" );

				fString( out, "\t}\n" );
			}

			// Extra functions
			fString( out, "\n" );
			fWriteSnippet( out, 1, data.mExtraFunctions );
			std::string extra = node.fExtraFunctions( );
			if( extra.length( ) > 0 )
			{
				fString( out, "\n" );
				fWriteSnippet( out, 1, extra );
			}
			data.mExtraFunctions = "";

			// End class
			fString( out, "}\n\n" );
		}
	}

	std::string tFile::fBuildScript( const std::string& uniqueFileName, b32 alreadyConstructed, const tDAGNodeCanvas::tDAGNodeList& onlyThese )
	{
		// collect data
		tGoamlData data;
		fCollect( NULL, data.mNodes, data.mConns, alreadyConstructed );
		data.mUniqueFileTag = uniqueFileName;

		std::string script;

		for( u32 i = 0; i < data.mNodes.fCount( ); ++i )
		{
			if( onlyThese.fCount( ) == 0 || onlyThese.fFind( data.mNodes[ i ] ) )
				fClass( script, data, i );
		}

		script += "\r\n\r\n";
		return script;
	}

}}

