#include "ToolsPch.hpp"
#include "DerivedAINodes.hpp"
#include "tDAGNodeCanvas.hpp"

#include "Editor/tEditablePropertyTypes.hpp"


namespace Sig
{

	namespace
	{
		const wxColour cColorValuesBarColorGoal ( 0xaa, 0x11, 0x88 ); //0x22, 0x19, 0xb2 );
		const wxColour cColorValuesTextColorGoal( 0xff, 0xff, 0x99 );

		const wxColour cColorValuesBarColorSwitch ( 0xb8, 0x00, 0x00 );
		const wxColour cColorValuesTextColorSwitch( 0xff, 0xff, 0x99 );

		const wxColour cColorValuesBarColorSequence ( 0x00, 0xba, 0xa6 );
		const wxColour cColorValuesTextColorSequence( 0xff, 0xff, 0xff );

		const wxColour cColorValuesBarColorLeaf ( 0xcc, 0xa3, 0x00 );
		const wxColour cColorValuesTextColorLeaf( 0x00, 0x00, 0x00 );
	}

	const std::string tGoalAINode::cActivateEventName = "~~OnActivate~~";
	const std::string tGoalAINode::cSuspendEventName = "~~OnSuspend~~";
	const std::string tGoalAINode::cCompleteEventName = "~~OnComplete~~";
	const std::string tGoalAINode::cFirstChanceEventName = "-First Chance-";

	const std::string tGoalAINode::cBasePropertiesName = "Goal Properties.Base Class";
	const std::string tGoalAINode::cOutputOverridePropertiesName = "Goal Properties.Output Class Name";
	const std::string tGoalAINode::cPriorityPropertiesName = "Goal Properties.Priority";
	const std::string tGoalAINode::cSingleInstancePropertiesName = "Goal Properties.SingleInstance";
	const std::string tGoalAINode::cMoStatePropertiesName = "Goal Properties Anim.Motion State";
	const std::string tGoalAINode::cOneShotPropertiesName = "Goal Properties Anim.OneShot";
	const std::string tGoalAINode::cWaitForEndPropertiesName = "Goal Properties Anim.WaitForEnd";
	const std::string tGoalAINode::cOneShotOverridePropertiesName = "Goal Properties Anim.TimerOverride";
	const std::string tGoalAINode::cExtraFunctionsPropertiesName = "Goal Properties.Extra Functions";
	const std::string tGoalAINode::cPersistPropertiesName = "Goal Properties.Persist";

	
	
	
	register_rtti_factory( tGoalAINode, false );	

	tGoalAINode::tGoalAINode( const wxPoint& p )
		: tAINode( "Goal", cColorValuesBarColorGoal, cColorValuesTextColorGoal, false, p )
		, mType( cGoalType )
		, mSystemHandlerCount( 0 )
	{
		fComputeDimensions( );
	}

	void tGoalAINode::fSetType( u32 type )
	{
		mType = type;

		fCheckProps( );

		if( mType == cGoalType )
		{
			fSetTitleBarColor( cColorValuesBarColorGoal );
			fSetTitleTextColor( cColorValuesTextColorGoal );
			fAddInput( " ", tDAGNodeOutput::cTop, wxColour( 0, 255, 0 ), "Push goal onto stack." );
			fAddInput( " ", tDAGNodeOutput::cTop, wxColour( 255, 255, 0 ), "Suspend this goal, not marked completed." );
			fAddInput( " ", tDAGNodeOutput::cTop, wxColour( 255, 0, 0 ), "Suspend this goal, marked completed." );

			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cOnActivateType, cActivateEventName, true, tDAGNodeOutput::cBottom, wxColour( 0, 255, 0 ) ) ) );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cOnSuspendType, cSuspendEventName, true, tDAGNodeOutput::cBottom, wxColour( 255, 255, 0 ) ) ) );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cOnCompleteType, cCompleteEventName, true, tDAGNodeOutput::cBottom, wxColour( 255, 0, 0 ) ) ) );
			mSystemHandlerCount = 3;

			mMoStateText.fReset( new tDAGNodeText( "MoState", "", *this, true ) );
			mTexts.fPushBack( mMoStateText );
		}
		else if( mType == cSwitchType )
		{
			fSetTitleBarColor( cColorValuesBarColorSwitch );
			fSetTitleTextColor( cColorValuesTextColorSwitch );
			fAddInput( " ", tDAGNodeOutput::cTop, wxColour( 0, 255, 0 ), "Initiate a goal push selection." );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cSwitchType, "", false, tDAGNodeOutput::cRight ) ) );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cSwitchType, "", false, tDAGNodeOutput::cRight ) ) );
			fSetTypeName( "Switch" );
			fSetName( fVerboseName( ) );
		}
		else if( mType == cSequenceType )
		{
			fSetTitleBarColor( cColorValuesBarColorSequence );
			fSetTitleTextColor( cColorValuesTextColorSwitch );
			fAddInput( " ", tDAGNodeOutput::cTop, wxColour( 0, 255, 0 ), "Initiate a goal push sequence." );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cSequenceType, "", false, tDAGNodeOutput::cRight ) ) );
			fAddEventHandler( tGoalEventHandlerPtr( new tGoalEventHandler( this, tGoalEventHandler::cSequenceType, "", false, tDAGNodeOutput::cRight ) ) );
			fSetTypeName( "Sequence" );
			fSetName( fVerboseName( ) );
		}

		fComputeDimensions( );
	}

	void tGoalAINode::fCheckProps( )
	{
		tGrowableArray< std::string > intendedProps;


		tEditableProperty::tDisplayOptions* options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyString( cNamePropertiesName, "" ) ) ))->fDisplayOptions( );
		options->mToolTip = "The name to display in the node canvas. Purely organizational.";
		options->mOrder = 1;
		intendedProps.fPushBack( cNamePropertiesName );

		if( mType == cGoalType )
		{
			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cPriorityPropertiesName, 0, 0, 100, 1, 0 ) ) ))->fDisplayOptions( );
			options->mToolTip = "The priority of this goal.";
			options->mOrder = 0;
			intendedProps.fPushBack( cPriorityPropertiesName );

			u32 order = 2;
			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyString( cBasePropertiesName, "" ) ) ))->fDisplayOptions( );
			options->mToolTip = "Name of a base goal to inherit.";
			options->mOrder = order++;
			intendedProps.fPushBack( cBasePropertiesName );

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyString( cOutputOverridePropertiesName, "" ) ) ))->fDisplayOptions( );
			options->mToolTip = "Leave blank to generate a unique goal name, or override with a specific one to reference elsewhere.";
			options->mOrder = order++;
			intendedProps.fPushBack( cOutputOverridePropertiesName );

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyScriptString( cExtraFunctionsPropertiesName, "" ) ) ))->fDisplayOptions( );
			options->mToolTip = "Enter any additional script functions here.";
			options->mOrder = order++;
			intendedProps.fPushBack( cExtraFunctionsPropertiesName );

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cSingleInstancePropertiesName, false ) ) ))->fDisplayOptions( );
			options->mToolTip = "Set checked to prevent this goal from showing up more than once in the stack.";
			options->mOrder = order++;
			intendedProps.fPushBack( cSingleInstancePropertiesName );

			// anim stuff
			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cOneShotPropertiesName, false ) ) ))->fDisplayOptions( );
			options->mToolTip = "Set checked to clear this goal when the animation has completed.";
			options->mOrder = order++;
			intendedProps.fPushBack( cOneShotPropertiesName );			

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cWaitForEndPropertiesName, false ) ) ))->fDisplayOptions( );
			options->mToolTip = "Clears goal from an animation event instead of a timer.  Requires OneShot to be enabled.";
			options->mOrder = order++;
			intendedProps.fPushBack( cWaitForEndPropertiesName );			

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( cMoStatePropertiesName, "" ) ) ))->fDisplayOptions( );
			options->mToolTip = "Choose a MoState from the browse button or enter a script variable/function here.";
			options->mOrder = order++;
			intendedProps.fPushBack( cMoStatePropertiesName );

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyString( cOneShotOverridePropertiesName, "" ) ) ))->fDisplayOptions( );
			options->mToolTip = "Override the life time of the goal here, with a number, variable, or function.";
			options->mOrder = order++;
			intendedProps.fPushBack( cOneShotOverridePropertiesName );

			options = &(*fAIProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cPersistPropertiesName, false ) ) ))->fDisplayOptions( );
			options->mToolTip = "Set checked to keep this goal around even with no subgoals.";
			options->mOrder = order++;
			intendedProps.fPushBack( cPersistPropertiesName );			
		}

		// removed old fields
		tEditablePropertyTable& table = fAIProps( );
		b32 done = false;
		while( !done )
		{
			done = true;
			tEditablePropertyTable::tIteratorNoNullOrRemoved it( table.fBegin( ), table.fEnd( ) );
			for( ; it != table.fEnd( ); ++it )
			{
				if( !intendedProps.fFind( it->mValue->fGetName( ) ) )
				{
					// shouldn't have this, erase it.
					table.fRemove( it->mKey );
					done = false;
					break;
				}
			}
		}
	}

	void tGoalAINode::fClearHandlers( )
	{
		fClearOutputs( );
	}

	void tGoalAINode::fAddEventHandler( tGoalEventHandlerPtr& handler, u32 mIndex )
	{
		s32 index = fIndexOfEventHandler( handler );
		if( index == -1 )
		{
			fAddEventHandlerToDAG( *handler, mIndex );
			handler->fRefreshName( );
		}
	}

	void tGoalAINode::fRemoveEventHandler( tGoalEventHandlerPtr& handler )
	{
		if( !handler->mLocked )
		{
			fRemoveEventHandlerFromDAG( *handler );
			mOwner->Refresh( );
		}
	}

	void tGoalAINode::fMoveEventHandler( tGoalEventHandlerPtr& handler, u32 mIndex )
	{
		if( !handler->mLocked )
		{
			fRemoveOutput( handler->mOutput );
			fAddOutput( handler->mOutput, mIndex + mSystemHandlerCount );
			for( u32 i = 0; i < fUserHandlerCount( ); ++i )
				fUserHandler( i )->fRefreshName( );

			mOwner->Refresh( );
		}
	}

	s32 tGoalAINode::fIndexOfEventHandler( const tGoalEventHandlerPtr& handler ) const
	{
		for( u32 i = 0; i < fUserHandlerCount( ); ++i )
		{
			tGoalEventHandler* h = fUserHandler( i );
			if( h && h == handler.fGetRawPtr( ) )
				return i;
		}

		return -1;
	}

	b32 tGoalAINode::fHasEventHandler( const std::string& name ) const
	{
		for( u32 i = 0; i < fUserHandlerCount( ); ++i )
		{
			tGoalEventHandler* h = fUserHandler( i );
			if( h && h->fType( ) == tGoalEventHandler::cRegularType && h->fName( ) == name )
				return true;
		}

		return false;
	}

	b32 tGoalAINode::fHasEventHandler( tGoalEventHandler::tType type ) const
	{
		for( u32 i = 0; i < fUserHandlerCount( ); ++i )
		{
			tGoalEventHandler* h = fUserHandler( i );
			if( h && h->fType( ) == type )
				return true;
		}

		return false;
	}

	void tGoalAINode::fFixHandler( tGoalEventHandler& handler )
	{
		// Dev Only - do some cleanup so files dont have to be recreated when the structure changes.
		//   ultimately this function should be empty, after all remaining unconverted files have been fixed up.
	}

	void tGoalAINode::fAddEventHandlerToDAG( tGoalEventHandler& handler, u32 mIndex )
	{
		fAddOutput( handler.mOutput, mIndex );
		fComputeDimensions( );
	}

	void tGoalAINode::fRemoveEventHandlerFromDAG( const tGoalEventHandler& handler )
	{
		sigassert(  handler.mOutput );
		fRemoveOutput( handler.mOutput );
		
		tDAGNodeCanvas::tDAGNodeConnectionList list;
		for( u32 i = 0; i < handler.mOutput->fConnectionList( ).fCount( ); ++i )
			list.fPushBack( tDAGNodeConnectionPtr( handler.mOutput->fConnectionList( )[ i ] ) );

		if( list.fCount( ) )
			mOwner->fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteConnectionAction( *mOwner, list, false ) ) );

		fComputeDimensions( );
	}

	std::string tGoalAINode::fVerboseName( ) const
	{
		std::string name = fDisplayableName( );

		if( mType == cGoalType )
		{
			u32 val;
			(*fAIProps( ).fFind( cPriorityPropertiesName ))->fGetData( val );
			name += " - " + StringUtil::fToString( val );
		}
		
		return name;
	}

	std::string tGoalAINode::fDisplayableName( ) const
	{
		std::string nameOverride = fNameOverride( );
		std::string baseName = ( mType == cGoalType ) ? fBaseClass( ) : "";
		std::string outputName = ( mType == cGoalType ) ? fOutputClassName( ) : "";
		std::string motionName = ( mType == cGoalType ) ? fMotionState( ) : "";
		std::string className;

		if( nameOverride.length( ) > 0 )
			className = nameOverride;
		else if( outputName.length( ) > 0 )
			className = outputName;
		else if( motionName.length( ) > 0 )
			className = StringUtil::fStripQuotes( StringUtil::fEatWhiteSpace( motionName ) );
		else if( baseName.length( ) > 0 )
			className = baseName;
		else 
			className = fTypeName( );

		return StringUtil::fEatWhiteSpace( className );
	}

	std::string tGoalAINode::fNameOverride( ) const 
	{
		std::string nameOverride;
		(*fAIProps( ).fFind( cNamePropertiesName ))->fGetData( nameOverride );

		nameOverride = StringUtil::fEatWhiteSpace( nameOverride );
		return nameOverride;
	}

	std::string tGoalAINode::fOutputClassName( ) const
	{
		std::string output;

		if( mType == cGoalType ) (*fAIProps( ).fFind( cOutputOverridePropertiesName ))->fGetData( output );

		output = StringUtil::fEatWhiteSpace( StringUtil::fReplaceAllOf( output, " ", "_" ) );
		return output;
	}

	std::string tGoalAINode::fBaseClass( ) const
	{
		std::string str;

		if( mType == cGoalType ) (*fAIProps( ).fFind( cBasePropertiesName ))->fGetData( str );

		return StringUtil::fEatWhiteSpace( str );
	}

	std::string tGoalAINode::fMotionState( ) const
	{
		std::string str;
		(*fAIProps( ).fFind( cMoStatePropertiesName ))->fGetData( str );
		return str;
	}

	b32 tGoalAINode::fOneShot( ) const
	{
		b32 val = false;
		(*fAIProps( ).fFind( cOneShotPropertiesName ))->fGetData( val );
		return val;
	}

	b32 tGoalAINode::fWaitForEndEvent( ) const
	{
		b32 val = false;
		(*fAIProps( ).fFind( cWaitForEndPropertiesName ))->fGetData( val );
		return val;
	}

	b32	tGoalAINode::fPersist( ) const
	{
		b32 val = false;
		(*fAIProps( ).fFind( cPersistPropertiesName ))->fGetData( val );
		return val;
	}

	std::string tGoalAINode::fOneShotOverrideScript( ) const
	{
		std::string str;
		(*fAIProps( ).fFind( cOneShotOverridePropertiesName ))->fGetData( str );
		return str;
	}

	std::string tGoalAINode::fExtraFunctions( ) const
	{
		std::string str;
		(*fAIProps( ).fFind( cExtraFunctionsPropertiesName ))->fGetData( str );
		return str;
	}

	b32 tGoalAINode::fSingleInstance( ) const
	{
		b32 val = false;
		(*fAIProps( ).fFind( cSingleInstancePropertiesName ))->fGetData( val );
		return val;
	}

	u32 tGoalAINode::fPriority( ) const
	{
		u32 val = 0;
		(*fAIProps( ).fFind( cPriorityPropertiesName ))->fGetData( val );
		return val;
	}

	void tGoalAINode::fSetPriority( u32 priority )
	{
		(*fAIProps( ).fFind( cPriorityPropertiesName ))->fSetData( priority );
		fApplyPropertyValues( );
	}

	void tGoalAINode::fApplyPropertyValues( )
	{
		fCheckProps( );

		// mostate stuff
		if( mMoStateText )
		{
			tEditablePropertyPtr* moState = fAIProps( ).fFind( cMoStatePropertiesName );
			tEditablePropertyPtr* oneShot = fAIProps( ).fFind( cOneShotPropertiesName );

			std::string val;
			std::string label = "MoState";

			if( moState && *moState )
				(*moState)->fGetData( val );

			if( oneShot && *oneShot )
			{
				b32 oneShotValue;
				(*oneShot)->fGetData( oneShotValue );
				if( oneShotValue ) label += " (1)";
			}

			mMoStateText->fSetLabel( label );
			mMoStateText->fSetValue( val );

			if( fType( ) == tGoalAINode::cGoalType )
			{
				if( val.length( ) > 0 )
				{
					fSetTitleBarColor( cColorValuesBarColorLeaf );
					fSetTitleTextColor( cColorValuesTextColorLeaf );
				}
				else
				{
					fSetTitleBarColor( cColorValuesBarColorGoal );
					fSetTitleTextColor( cColorValuesTextColorGoal );
				}
			}

			fComputeDimensions( );
		}

		{
			fSetName( fVerboseName( ) );
			fComputeDimensions( );
		}
	}


	/////////////////////////////////////////////////////
	// tGoalEventHandler stuff

	const std::string tGoalEventHandler::cNamePropertyName = "Handler Setup.Name";
	const std::string tGoalEventHandler::cTypePropertyName = "Handler Setup.Type";
	const std::string tGoalEventHandler::cPollFreqPropertyName = "Handler Setup.Poll Frequency";
	const std::string tGoalEventHandler::cScriptPropertyName = "Handler Setup.Script";

	// Common text to all handlers
#define EventHandlerHeader \
	"Symbols available to you:\n" \
	"\t Any variables declared in the goal variable snippet, any functions/objects/constants exported to script.\n" \
	"\n"

#define EventHandlerStaticTextFire \
	"\t logic - This is your full logic object.\n " \
	"\t params - This is the param table used to intialize the goal.\n" \
	"\n" \
	"\t fire( [params table] ) - Will follow the connection to switch goals based on the connections behavior.\n" \
	"\t\tUsing \"fire\" will assume you intended to pass the param table as the [params table] parameter.\n" \
	"\t\tHaving no fire commands in the snippet at all, assumes you meant to have a \"fire( params )\" before the last return found.\n"

	const char tGoalEventHandler::cEventHandlerStaticText[] = 
	{
		EventHandlerHeader
		"\t event - This is the logic event object. It has properties: Id, Context\n"
		EventHandlerStaticTextFire
		"\t return true - Indicates that the event has been handled and no other goal lower in the stack should handle the event.\n"
		"\t Base EventContext types: StringEventContext, IntEventContext, ObjectEventContext.\n"
		"\t  Ex: local context = StringEventContext.Convert( event.Context ); print( context.String )"
	};

	// Same as above less the event symbol.
	const char tGoalEventHandler::cEventHandlerStaticTextLessEvent[] = 
	{
		EventHandlerHeader
		EventHandlerStaticTextFire
	};

	tGoalEventHandler::tGoalEventHandler( tDAGNode* owner, tType type, const std::string& name, b32 locked, tDAGNodeOutput::tEdge edge, const wxColour& color )
		: mLocked( locked )
		, mEdge( edge ), mColor( color )
	{
		fCheckProps( );

		(*mProps.fFind( cNamePropertyName ))->fSetData( name );
		(*mProps.fFind( cTypePropertyName ))->fSetData( u32( type ) );

		fReadProps( );

		fMakeOutput( owner );
	}

	tGoalEventHandler::tGoalEventHandler( const tEditablePropertyTable& props )
		: mProps( props )
	{
		fCheckProps( );
	}

	tGoalEventHandler::tGoalEventHandler( ) 
	{ 
		fCheckProps( );
	}

	tGoalEventHandler::tType tGoalEventHandler::fType( ) const 
	{ 
		u32 t;
		(*mProps.fFind( cTypePropertyName ))->fGetData( t );
		return (tType)t; 
	}

	void tGoalEventHandler::fSetType( tType type )
	{
		(*mProps.fFind( cTypePropertyName ))->fSetData( u32(type) );
	}

	std::string tGoalEventHandler::fName( ) const
	{
		std::string name;
		(*mProps.fFind( cNamePropertyName ))->fGetData( name );

		if( fType( ) == cRegularType )
		{
			u32 key = atoi( name.c_str( ) );
			const tProjectFile::tEvent* e = tProjectFile::fGetCurrentProjectFileCached( ).fFindGameEventByKey( key );

			if( !e ) 
			{
				log_warning( 0, "Could not find game event with key : " << key );
				name = "Error!";
			}
			else 
				name = e->mName;
		}

		return name; 
	}

	u32 tGoalEventHandler::fPollFreq( ) const
	{
		u32 f;
		(*mProps.fFind( cPollFreqPropertyName ))->fGetData( f );
		return f; 
	}

	void tGoalEventHandler::fSetPollFreq( u32 freq )
	{
		(*mProps.fFind( cPollFreqPropertyName ))->fSetData( freq );
	}

	std::string tGoalEventHandler::fStaticScriptText( ) const
	{
		return std::string( (!fLocked( ) && fType( ) == tGoalEventHandler::cRegularType) ? cEventHandlerStaticText : cEventHandlerStaticTextLessEvent );
	}

	std::string tGoalEventHandler::fScript( ) const
	{
		std::string script;
		(*mProps.fFind( cScriptPropertyName ))->fGetData( script );
		return script; 
	}

	std::string tGoalEventHandler::fFirstScriptLine( ) const
	{
		tGrowableArray<std::string> lines;
		std::string script = fScript( );
		StringUtil::fSplit( lines, script.c_str( ), "\n" );
		return lines.fCount( ) > 0 ? lines[ 0 ] : "";
	}

	void tGoalEventHandler::fSetScript( const std::string& script )
	{
		(*mProps.fFind( cScriptPropertyName ))->fSetData( script );
		if( mOutput ) mOutput->fSetName( fOutputName( ) );
	}

	b32 tGoalEventHandler::fHasData( ) const 
	{ 
		return fScriptValid( ); 
	}

	void tGoalEventHandler::fCheckProps( )
	{
		(*mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyString( cNamePropertyName ) ) ))->fDisplayOptions( ).mShow = false;

		tDynamicArray< std::string > types;
		types.fInsert( 0, cTypeStrings, cTypeCount );
		(*mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cTypePropertyName, types, 0 ) ) ))->fDisplayOptions( ).mShow = false;

		tDynamicArray< std::string > freqs;
		freqs.fInsert( 0, cPollFreqStrings, cPollFreqCount );
		(*mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cPollFreqPropertyName, freqs, cPollTenthSecond ) ) ))->fDisplayOptions( ).mShow = false;

		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyScriptString( cScriptPropertyName ) ) );
	}

	void tGoalEventHandler::fFillProps( )
	{
		tEditablePropertyScriptString* scriptProp = static_cast< tEditablePropertyScriptString* >( mProps.fFind( cScriptPropertyName )->fGetRawPtr( ) );
		scriptProp->fSetStaticText( fStaticScriptText( ) );
	}

	void tGoalEventHandler::fReadProps( )
	{
		// Loading them will not preserve their show value
		(*mProps.fFind( cNamePropertyName ))->fDisplayOptions( ).mShow = (fType( ) == cPolledType) || (fType( ) == cSequenceType) || (fType( ) == cSwitchType);
		(*mProps.fFind( cTypePropertyName ))->fDisplayOptions( ).mShow = false;
		(*mProps.fFind( cPollFreqPropertyName ))->fDisplayOptions( ).mShow = ( fType( ) == cPolledType );

		if( mOutput ) 
		{
			mOutput->fSetName( fOutputName( ) );

			wxString toolTip = "Event Handler. Left or Right click for options.";
			switch( fType( ) )
			{
			case cOnActivateType: toolTip = "On Activate Snippet. Right click for options."; break;
			case cOnSuspendType: toolTip = "On Suspend Snippet. Right click for options."; break;
			case cOnCompleteType: toolTip = "On Complete Snippet. Right click for options."; break;
			}

			mOutput->fSetToolTip( toolTip );
		}
	}

	void tGoalEventHandler::fMakeOutput( tDAGNode* owner )
	{
		if( owner )
		{
			mOutput.fReset( new tDAGNodeOutput( 
				fOutputName( ),
				*owner, 
				mColor,
				1,
				(tDAGNodeOutput::tEdge) mEdge ) );
			mOutput->fSetData( tDAGDataPtr( this ) );
		}
	}

	std::string tGoalEventHandler::fOutputName( ) const
	{
		tType type = fType( );
		if( type == cPolledType ) 
			return "Polled - " + fName( );
		if( type == cPotentialType ) 
			return "-Potential-";
		else if( type == cFirstChanceType ) 
			return "-First Chance-";
		else if( type == cSwitchType || type == cSequenceType )
		{
			std::string name = fName( );
			if( name.length( ) > 0 ) 
				return name;
			else if( fScriptValid( ) ) 
				return fFirstScriptLine( );
			else if( type == cSwitchType )
				return "*Default*";
			else if( type == cSequenceType && mOutput )
				return StringUtil::fToString( mOutput->fIndex( ) + 1 );
			else 
				return "Item";
		}
		else
			return fName( );
	}


	const std::string tGoalEventHandler::cTypeStrings[ cTypeCount ] =
	{ "Regular", "Polled", "Potential", "First Chance" };

	const std::string tGoalEventHandler::cPollFreqStrings[ cPollFreqCount ] =
	{ "Every frame", "10 times per second", "4 times per second", "2 times per second", "1 time per second", "Every 2 seconds", "Every 5 seconds" };


	void fSerializeXmlObject( tXmlSerializer& s, wxColour &color )
	{
		u32 r = color.Red( );
		u32 g = color.Green( );
		u32 b = color.Blue( );
		u32 a = color.Alpha( );
		s( "R", r );
		s( "G", g );
		s( "B", b );
		s( "A", a );
	}
	void fSerializeXmlObject( tXmlDeserializer& s, wxColour &color )
	{
		u32 r, g, b, a;
		s( "R", r );
		s( "G", g );
		s( "B", b );
		s( "A", a );

		color = wxColour( r,g,b,a );
	}


	//////////////////////////////////////////////////////////
	//  Connection data

	const std::string tAIConnectionData::cConnectionPosition = "Connection Setup.Position";
	const std::string tAIConnectionData::cConnectionBehavior = "Connection Setup.Behavior";
	const std::string tAIConnectionData::cConnectionMaxClear = "Connection Setup.Max Clear";
	const std::string tAIConnectionData::cBehaviorNames[ tAIConnectionData::cBehaviorCount ] = 
	{
		"Switch To State",
		"Push State on Top",
		"Clear then Push on Top",
	};

	const u32 tAIConnectionData::cBehaviorListOrder[ tAIConnectionData::cBehaviorCount ] = 
	{ 
		tAIConnectionData::cBehaviorPush,
		tAIConnectionData::cBehaviorClearAndPush,
		tAIConnectionData::cBehaviorSwitch
	};

	u32 tAIConnectionData::fBehaviorListIndex( u32 value )
	{
		for( u32 i = 0; i < tAIConnectionData::cBehaviorCount; ++i )
			if( cBehaviorListOrder[ i ] == value ) return i;

		return -1;
	}

	u32 tAIConnectionData::fBehaviorFromIndex( u32 index )
	{
		sigassert( index < tAIConnectionData::cBehaviorCount );
		return cBehaviorListOrder[ index ];
	}


	tAIConnectionData::tAIConnectionData( ) 
	{
		fCheckProps( );
	}

	void tAIConnectionData::fCheckProps( )
	{ 
		// ensure we have the needed props
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cConnectionPosition, 30.f, -1000.0f, 1000.f, 0.1f, 1 ) ) );

		tDynamicArray< std::string > behaves;
		behaves.fInsert( 0, cBehaviorNames, cBehaviorCount );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cConnectionBehavior, behaves, cDefaultBehavior ) ) );

		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cConnectionMaxClear, 0, 0, 100, 1, 0 ) ) );

		// removed old fields
		tEditablePropertyTable& table = mProps;

		b32 done = false;
		while( !done )
		{
			done = true;
			tEditablePropertyTable::tIteratorNoNullOrRemoved it( table.fBegin( ), table.fEnd( ) );
			for( ; it != table.fEnd( ); ++it )
			{
				const std::string& name = it->mValue->fGetName( );
				if( name == cConnectionPosition ) continue;
				else if( name == cConnectionBehavior ) continue;
				else if( name == cConnectionMaxClear ) continue;

				// shouldn't have this erase it.
				table.fRemove( it->mKey );
				done = false;
				break;
			}
		}
	}

	tAIConnectionData::tAIConnectionData( const tEditablePropertyTable& props ) 
		: mProps( props )
	{ 
		fCheckProps( );
	}

	void tAIConnectionData::fFillProps( )
	{
	}

	void tAIConnectionData::fReadProps( )
	{
		std::string name;
		if( fBehavior( ) != cBehaviorSwitch )
		{
			if( name.length( ) > 0 ) name += " - ";
			name += StringUtil::fToString( fMaxClearablePriority( ) );
		}

		if( mConnection->fInput( )->fIndex( ) != tGoalAINode::cInputActivate )
			name = "";

		const tGoalAINode* inputEnd = dynamic_cast< const tGoalAINode* >( &mConnection->fInput( )->fOwner( ) );
		if( inputEnd && (inputEnd->fType( ) == tGoalAINode::cSequenceType || inputEnd->fType( ) == tGoalAINode::cSwitchType ) )
			name = "";

		mConnection->fSetText( name );

		f32 pos;
		(*mProps.fFind( cConnectionPosition ))->fGetData( pos );
		mConnection->fSetTextPosition( pos );

		fSetConnectionColor( );
	}

	tAIConnectionData::tBehavior tAIConnectionData::fBehavior( ) const
	{
		u32 val = 0;
		(*mProps.fFind( cConnectionBehavior ))->fGetData( val );
		return tBehavior( val );
	}

	u32 tAIConnectionData::fMaxClearablePriority( ) const
	{
		u32 val = 0;
		(*mProps.fFind( cConnectionMaxClear ))->fGetData( val );
		return val;
	}

	void tAIConnectionData::fSetMaxClearablePriority( u32 priority )
	{
		(*mProps.fFind( cConnectionMaxClear ))->fSetData( priority );
		fReadProps( );
	}	

	void tAIConnectionData::fSetConnectionColor( )
	{
		const tGoalAINode* inputEnd = dynamic_cast< const tGoalAINode* >( &mConnection->fInput( )->fOwner( ) );
		if( inputEnd && (inputEnd->fType( ) == tGoalAINode::cSequenceType || inputEnd->fType( ) == tGoalAINode::cSwitchType ) )
		{
			mConnection->fSetColor( tDAGNodeConnection::gDefaultConnectionColor );
			mConnection->fSetToolTip( "This connection will not push a goal itself." );
		}
		else if( mConnection->fInput( )->fIndex( ) != tGoalAINode::cInputActivate )
		{
			////this connection will terminate a goal, take on color of input
			//connection->fSetColor( connection->fInput( )->fColor( ) );
			mConnection->fSetColor( tDAGNodeConnection::gDefaultConnectionColor );
			mConnection->fSetToolTip( "This connection will terminate a goal." );
		}
		else
		{
			u32 behave = fBehavior( );

			switch( behave )
			{
				// switching removes the current state, and is therefore usually not desirable, flag it red
			case tAIConnectionData::cBehaviorSwitch: 
				mConnection->fSetColor( wxColor( 255, 0, 0 ) ); 
				mConnection->fSetToolTip( "This connection will replace a goal with another." );
				break;
				// clearing is also slightly not desirable, so flag it yellow
			case tAIConnectionData::cBehaviorClearAndPush: 
				mConnection->fSetColor( wxColor( 255, 255, 0 ) ); 
				mConnection->fSetToolTip( "This connection will clear and then push a goal." );
				break;
			default:
				mConnection->fSetColor( wxColor( 0, 255, 0 ) );
				mConnection->fSetToolTip( "This connection will push a goal." );
			}
		}
	}



}

