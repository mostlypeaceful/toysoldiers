#ifndef __DerivedAINodes__
#define __DerivedAINodes__
#include "tAiNode.hpp"
#include "ai/tGoal.hpp"

namespace Sig
{


	void fSerializeXmlObject( tXmlSerializer& s, wxColour &color );
	void fSerializeXmlObject( tXmlDeserializer& s, wxColour &color );

	struct tools_export tGoalEventHandler : public tDAGData
	{
	public:

		enum tType 
		{ 
			cRegularType, 
			cPolledType, 
			cPotentialType, 
			cFirstChanceType, 
			cOnActivateType, 
			cOnSuspendType, 
			cOnCompleteType,
			cSwitchType, 
			cSequenceType,
			cOnTickType,
			cAIFlagEvent,
			cTypeCount 
		};

		static const std::string cTypeStrings[ cTypeCount ];
		enum tPollFreq { cPollEverFrame, cPollTenthSecond, cPollQuarterSecond, cPollHalfSecond, cPollOneSecond, cPollTwoSeconds, cPollFiveSeconds, cPollFreqCount };
		static const std::string cPollFreqStrings[ cPollFreqCount ];

		static const std::string cNamePropertyName;
		static const std::string cTypePropertyName;
		static const std::string cPollFreqPropertyName;
		static const std::string cScriptPropertyName;
		static const char cEventHandlerStaticText[];
		static const char cEventHandlerStaticTextLessEvent[];

		tEditablePropertyTable mProps;

		tDAGNodeOutputPtr	mOutput;
		b32					mLocked;
		u32					mEdge;
		wxColour			mColor;
		u32					mAIFlagKey;
		b32					mAIFlagGoesTrue;

		tGoalEventHandler( );
		tGoalEventHandler( tDAGNode* owner, tType type, const std::string& name, b32 locked, tDAGNodeOutput::tEdge edge = tDAGNodeOutput::cRight, const wxColour& color = wxColour( 255, 255, 255 ) );
		tGoalEventHandler( const tEditablePropertyTable& props );

		tType		fType( ) const;
		void		fSetType( tType type );
		std::string fName( ) const;
		std::string fOutputName( ) const;
		b32			fLocked( ) const { return mLocked; }
		u32			fPollFreq( ) const;
		void		fSetPollFreq( u32 freq );

		std::string fAIFlagCheckEventID( ) const;
		std::string fAIFlagIndex( ) const;

		std::string fStaticScriptText( ) const;
		std::string fScript( ) const;
		b32			fScriptValid( ) const { return fScript( ).length( ) > 0; }
		std::string fFirstScriptLine( ) const;
		void		fSetScript( const std::string& script );

		void fMakeOutput( tDAGNode* owner );

		void fCommonInit( );
		void fCheckProps( );
		void fFillProps( );
		void fReadProps( );
		void fRefreshName( ) { if( mOutput ) mOutput->fSetName( fOutputName( ) ); }

		virtual b32 fHasData( ) const;

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "Locked", mLocked );
			s( "Edge", mEdge );
			s( "Color", mColor );
			s( "Props", mProps );
			s( "AIFlag", mAIFlagKey );
			s( "AIFlagGT", mAIFlagGoesTrue );
		}
	};
	typedef tRefCounterPtr< tGoalEventHandler > tGoalEventHandlerPtr;

	struct tMoStateReference
	{
		tMoStateReference( const std::string& name = "" )
			: mName( name )
		{ }

		std::string mName;

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "Name", mName );
		}
	};

	class tools_export tGoalAINode : public tAINode
	{
	public:
		implement_derived_ai_node( tGoalAINode, 0xB931353E )

		virtual void fSerialize( tXmlSerializer& s ) 
		{ 
			tGrowableArray< tGoalEventHandlerPtr > eventHandlers;
			for( u32 i = 0; i < fHandlerCount( ); ++i ) 
			{
				tGoalEventHandler* handler = fHandler( i );
				sigassert( handler );
				handler->fFillProps( );
				eventHandlers.fPushBack( tGoalEventHandlerPtr( handler ) );
			}

			s( "Type", mType );
			s( "Events", eventHandlers ); 
			s( "MoStates", mMoStateReferences );
		}
		virtual void fSerialize( tXmlDeserializer& s ) 
		{ 
			tGrowableArray< tGoalEventHandlerPtr > eventHandlers;
			s( "Type", mType );
			fSetType( mType );

			s( "Events", eventHandlers );
			s( "MoStates", mMoStateReferences );
			
			fClearHandlers( );

			for( u32 i = 0; i < eventHandlers.fCount( ); ++i ) 
			{
				fFixHandler( *eventHandlers[ i ] );
				eventHandlers[ i ]->fMakeOutput( this );
				fAddEventHandler( eventHandlers[ i ] );
				eventHandlers[ i ]->fReadProps( );
			}

			fFixGoal( );
		}

		enum tType { cGoalType, cSwitchType, cSequenceType, cTypeCount };
		enum tInput { cInputActivate, cInputSuspend, cInputTerminate, cInputCount };

		static const std::string cMoStatePropertiesName;
		static const std::string cOneShotPropertiesName;
		static const std::string cBasePropertiesName;
		static const std::string cOutputOverridePropertiesName;
		static const std::string cOutputClassName;
		static const std::string cPriorityPropertiesName;
		static const std::string cSingleInstancePropertiesName;
		static const std::string cOneShotOverridePropertiesName;
		static const std::string cExtraFunctionsPropertiesName;
		static const std::string cPersistPropertiesName;

		static const std::string cActivateEventName;
		static const std::string cSuspendEventName;
		static const std::string cCompleteEventName;
		static const std::string cTickEventName;
		static const std::string cFirstChanceEventName;
	public:
		explicit tGoalAINode( const wxPoint& p = wxPoint( 0, 0 ) );

		void fClearHandlers( );
		void fAddEventHandler( tGoalEventHandlerPtr& handler, u32 mIndex = ~0 );
		void fRemoveEventHandler( tGoalEventHandlerPtr& handler );
		void fMoveEventHandler( tGoalEventHandlerPtr& handler, u32 mIndex );
		s32 fIndexOfEventHandler( const tGoalEventHandlerPtr& handler ) const;
		b32 fHasEventHandler( const std::string& name ) const;
		b32 fHasEventHandler( tGoalEventHandler::tType type ) const;

		u32 fUserHandlerCount( ) const { return fMax<s32>( 0, mOutputs.fCount( ) - mSystemHandlerCount ); }
		tGoalEventHandler *fUserHandler( u32 index ) const { return dynamic_cast< tGoalEventHandler* >( mOutputs[ index + mSystemHandlerCount ]->fData( ).fGetRawPtr( ) ); }
		u32 fHandlerCount( ) const { return mOutputs.fCount( ); }
		tGoalEventHandler *fHandler( u32 index ) const { return dynamic_cast< tGoalEventHandler* >( mOutputs[ index ]->fData( ).fGetRawPtr( ) ); }

		std::string fVerboseName( ) const;
		std::string fOutputClassName( ) const;
		std::string fNameOverride( ) const;
		std::string fDisplayableName( ) const;
		std::string fBaseClass( ) const;
		std::string fMotionState( ) const;
		b32			fOneShot( ) const;
		b32			fPersist( ) const;
		std::string fOneShotOverrideScript( ) const;
		std::string fExtraFunctions( ) const;
		b32			fSingleInstance( ) const;
		u32			fPriority( ) const;
		void		fSetPriority( u32 priority );
		tType		fType( ) const { return tType( mType ); }
		void		fSetType( u32 type );

		void		fFixSystemHandlers( tGrowableArray<tDAGNodeConnectionPtr> & connList );

		virtual void fApplyPropertyValues( );

		tGrowableArray<tMoStateReference> mMoStateReferences; // only applicable for goals. references into the blend states of a the MoMap to be activated with this goal. (instead of the old MoState functions system.)

	private:
		tDAGNodeTextPtr mMoStateText;
		tDAGNodeTextPtr mClearBehaviorText;
		u32 mType;
		s32 mSystemHandlerCount;

		void fCheckProps( );
		void fFixGoal( );
		void fFixHandler( tGoalEventHandler& handler );
		void fAddEventHandlerToDAG( tGoalEventHandler& handler, u32 mIndex = ~0 );
		void fRemoveEventHandlerFromDAG( const tGoalEventHandler& handler );

		// System Event Handlers
		void fAddOnActivateHandler( );
		void fAddOnSuspendHandler( );
		void fAddOnCompleteHandler( );
		void fAddOnTickHandler( );
	};
	typedef tRefCounterPtr< tGoalAINode > tGoalAINodePtr;


	class tools_export tAIConnectionData : public tDAGData
	{
	public:
		enum tBehavior
		{
			cBehaviorSwitch,
			cBehaviorPush,
			cBehaviorClearAndPush,
			cBehaviorPushBelow,
			cBehaviorCount
		};
		static const u32 cDefaultBehavior = cBehaviorPush;

		tEditablePropertyTable mProps;
		tDAGNodeConnectionPtr mConnection;

		static const std::string cConnectionPosition;
		static const std::string cConnectionBehavior;
		static const std::string cConnectionMaxClear;

		static const std::string cBehaviorNames[ cBehaviorCount ];
		static const u32 cBehaviorListOrder[ cBehaviorCount ];
		static u32 fBehaviorListIndex( u32 value );
		static u32 fBehaviorFromIndex( u32 index );

		tAIConnectionData( );
		tAIConnectionData( const tEditablePropertyTable& props );
		void fSetConnection( const tDAGNodeConnectionPtr& connection ) { mConnection = connection; }

		void fCheckProps( );
		void fFillProps( );
		void fReadProps( );

		tBehavior fBehavior( ) const;
		u32 fMaxClearablePriority( ) const;
		void fSetMaxClearablePriority( u32 priority );

		void fSetConnectionColor( );
	};
}


#endif//__DerivedAINodes__
