#ifndef __tDAGNode__
#define __tDAGNode__

namespace Sig
{
	class tDAGNodeCanvas;

	class tDAGNode;
	typedef tRefCounterPtr<tDAGNode> tDAGNodePtr;

	class tDAGNodeConnection;
	typedef tRefCounterPtr<tDAGNodeConnection> tDAGNodeConnectionPtr;

	/// Derive from this for extra storage data in the nodes and outputs.
	class tools_export tDAGData : public tRefCounter
	{
	public:
		virtual ~tDAGData( ) { }
		virtual b32 fHasData( ) const { return true; }
	};
	typedef tRefCounterPtr<tDAGData> tDAGDataPtr;

	/// Common base class for all DAG objects
	class tools_export tDAGObject : public tRefCounter
	{
	protected:
		tDAGDataPtr	mData;
		b32			mSelected;
		wxString	mToolTip;

	public:
		tDAGObject( ) : mSelected( false ) { }
		virtual ~tDAGObject( ) { }

		void fSetData( const tDAGDataPtr& data ) { mData = data; }
		const tDAGDataPtr& fData( ) const { return mData; }

		void fSetSelected( b32 sel ) { mSelected = sel; }
		b32 fSelected( ) const { return mSelected; }	

		void fSetToolTip( const wxString& toolTip ) { mToolTip = toolTip; }
		const wxString& fToolTip( ) const { return mToolTip; }
	};
	typedef tRefCounterPtr<tDAGObject> tDAGObjectPtr;

	/// Output connection, also serves as base for DAG input connection
	class tools_export tDAGNodeOutput : public tDAGObject
	{
	public:
		typedef tGrowableArray<tDAGNodeConnection*> tConnectionList;

		enum tIO { cOutput, cInput };
		enum tEdge { cLeft, cTop, cRight, cBottom, cEdgeCount };

	protected:
		tDAGNode& mOwner;
		tConnectionList mConnections;
		wxColour mColor;
		wxColour mTextColor;
		wxBrush mFillBrush, mFillConnectedBrush;
		wxPen mOutlinePen;
		tIO mIO;
		wxPoint mParentOffset;
		f32 mRadius;
		f32 mCircleXOffset;
		f32 mAnchorXOffset;
		u32 mMaxConnections;
		tEdge mEdge;

		std::string mName;
		f32 mLabelXOffset;
	public:
		explicit tDAGNodeOutput( const std::string& name, tDAGNode& owner, const wxColour& color, u32 maxConnections = ~0, tEdge edge = cRight, f32 radius = 6.5f );

		wxString fName( ) const { return wxString( mName.c_str( ) ); }
		void fSetName( const std::string& name ) { mName = name; }

		tDAGNode& fOwner( ) { return mOwner; }
		const tDAGNode& fOwner( ) const { return mOwner; }

		const wxColor& fColor( ) const { return mColor; }
		tEdge fEdge( ) const { return mEdge; }
		void fSetNameColor( const wxColour& color ) { mTextColor = color; }

		b32 fHasConnection( tDAGNodeConnection* connection ) const;
		void fAddConnection( tDAGNodeConnection* connection );
		void fRemoveConnection( tDAGNodeConnection* connection );
		void fClearConnections( );
		b32 fMaxConnectionsReached( ) const { return mConnections.fCount( ) == mMaxConnections; }
		const tConnectionList& fConnectionList( ) const { return mConnections; }

		void fOnPaint( wxPoint parentTopLeft, f32 offsets[cEdgeCount], wxDC& dc );
		Math::tVec2f fGetCenterPoint( ) const { return Math::tVec2f( mParentOffset.x + mCircleXOffset + mRadius, mParentOffset.y + mRadius ); }
		wxPoint fGetAnchorPoint( ) const;
		wxPoint fGetConnectionNormal( f32 length ) const;
		b32 fContains( const wxPoint& p ) const;
		b32 fIntersects( const wxPoint& parentTopLeft, const wxRect& rect ) const;
		wxRect fCircleRect( const wxPoint& parentTopLeft ) const;
		u32 fHeight( ) const;
		virtual u32 fIndex( ) const;
	};
	typedef tRefCounterPtr<tDAGNodeOutput> tDAGNodeOutputPtr;

	class tools_export tDAGNodeInput : public tDAGNodeOutput 
	{ 
	public:
		explicit tDAGNodeInput( const std::string& name, tDAGNode& owner, const wxColour& color, u32 maxConnections = ~0, tEdge edge = cLeft, f32 radius = 6.5f );
		virtual u32 fIndex( ) const;
	};
	typedef tRefCounterPtr<tDAGNodeInput> tDAGNodeInputPtr;

	class tools_export tDAGNodeText : public tDAGObject
	{
	protected:
		tDAGNode& mOwner;
		wxPoint mParentOffset;

		std::string mName;
		std::string mValue;
		f32 mLabelXOffset;
		b32 mHideIfNullText;
	public:
		explicit tDAGNodeText( const std::string& name, const std::string& value, tDAGNode& owner, b32 hideIfNullText );
		void fOnPaint( wxPoint parentTopLeft, f32 offsets[tDAGNodeOutput::cEdgeCount], wxDC& dc );
		const tDAGNode& fOwner( ) const { return mOwner; }
		u32 fHeight( ) const;

		void fSetLabel( const std::string& label ) { mName = label; }
		void fSetValue( const std::string& value ) { mValue = value; }
		wxString fTotalString( ) const;
	};
	typedef tRefCounterPtr<tDAGNodeText> tDAGNodeTextPtr;

	class tools_export tDAGNode : public tDAGObject
	{
	public:
		typedef tGrowableArray<tDAGNodeTextPtr> tDAGNodeTextList;
		typedef tGrowableArray<tDAGNodeInputPtr> tDAGNodeInputList;
		typedef tGrowableArray<tDAGNodeOutputPtr> tDAGNodeOutputList;
	protected:
		tDAGNodeCanvas* mOwner;
		std::string mName;
		wxPoint mTopLeft;
		wxSize mDimensions;
		wxBrush mTitleBarBrush;
		wxColour mTitleBarTextColor;
		int mRoundedRadius;
		int mTitleBarHeight;
		int mTitleBarTextXOffset;
		int mTitleBarTextYOffset;
		int mDropShadowOffset;
		int mCloseBoxWidth;
		int mCloseBoxXOffset;
		int mCloseBoxYOffset;
		int mCloseBoxHeight;
		s32 mBaseConnectionOffset;
		tDAGNodeTextList mTexts;
		tDAGNodeOutputList mOutputs;
		tDAGNodeInputList mInputs;
	public:
		enum tAction
		{
			cActionNone,
			cActionDelete,
			cActionBeginDrag,
			cActionBeginConnection,
			cActionRedoConnection,
			cActionOutputSelect,
			cActionUserNode,
			cActionUserOutput,
			cActionCount
		};
		struct tActionContext
		{
			tDAGNodeOutputPtr mOutput; // relevant for cActionBeginConnection
			tDAGNodeInputPtr mInput; // relevant for cActionBeginConnection and cActionRedoConnection
			tDAGNodeConnectionPtr mConnection; // relevant for cActionRedoConnection
		};

		enum tRenderStyle
		{
			cRenderInputsBeforeOutputs,
			cRenderOutputsBeforeInputs,
			cRenderOutputsNextToInputs
		};

		static const s32 cLineSpacing = 17;
	public:
		explicit tDAGNode( const std::string& name, const wxColour& titleBarColor, const wxColour& titleBarTextColor, const wxPoint& topLeft = wxPoint( 0, 0 ) );
		virtual ~tDAGNode( );
		
		void fSetOwner( tDAGNodeCanvas* owner ) { mOwner = owner; }
		tDAGNodeCanvas* fOwner( ) const { sigassert( mOwner && "Owner hasn't been set." ); return mOwner; }
		void fOnPaint( const wxPoint& topLeftOrigin, const wxSize& clipDims, wxDC& dc, tRenderStyle renderStyle );

		const std::string& fName( ) const { return mName; }
		void fSetName( const std::string& name );
		virtual wxString fDisplayName( ) const { return wxString( mName.c_str( ) ); }

		void fSetTitleBarColor( const wxColour& color ) { mTitleBarBrush = color; }
		void fSetTitleTextColor( const wxColour& color ) { mTitleBarTextColor = color; }
		void fSetPosition( const wxPoint& pos );
		const wxPoint& fPosition( ) const { return mTopLeft; }
		void fComputeDimensions( );
		const wxSize& fDimensions( ) const { return mDimensions; }
		void fMove( const wxPoint& delta );

		virtual tAction fOnLeftButtonDown( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut );
		virtual tAction fOnRightButtonDown( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut ) { return cActionNone;  }
		virtual tAction fOnLeftButtonUp( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut ) { return cActionNone;  }
		virtual tAction fOnRightButtonUp( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut );

		b32 fTryToCompleteConnection( const wxPoint& p, tDAGNodeConnection& connection, b32 preventCycles );
		void fCollectAllConnections( tDAGNodeOutput::tConnectionList& connections ) const;
		b32 fConnectsTo( const tDAGNode& node ) const;

		b32 fContains( const wxPoint& canvasP ) const;
		b32 fIntersects( const wxRect& rect ) const;
		const wxPoint& fTopLeft( ) const { return mTopLeft; }

		const u32 fInputCount( ) const { return mInputs.fCount( ); }
		const tDAGNodeInputPtr& fInput( u32 i ) const { return mInputs[ i ]; }
		tDAGNodeInputPtr fInput( const std::string& name ) const;
		const u32 fInputIndex( const tDAGNodeInput* of ) const;
		const u32 fOutputCount( ) const { return mOutputs.fCount( ); }
		const tDAGNodeOutputPtr& fOutput( u32 i ) const { return mOutputs[ i ]; }
		tDAGNodeOutputPtr fOutput( const std::string& name ) const;
		const u32 fOutputIndex( const tDAGNodeOutput* of ) const;

		void fClearOutputs( );

	};


	class tools_export tDAGNodeConnection : public tDAGObject
	{
		tDAGNodeInputPtr mInput;
		tDAGNodeOutputPtr mOutput;
		wxPoint mStartPos, mEndPos;
		std::string mText;
		f32 mTextPosition;
		wxColor mColor;

		void fGetPoints( const wxPoint& topLeftOrigin, tGrowableArray<wxPoint>& points, b32 doSpline );
	public:
		tDAGNodeConnection( const tDAGNodeInputPtr& start, const wxPoint& currentPos );
		tDAGNodeConnection( const tDAGNodeOutputPtr& start, const wxPoint& currentPos );
		tDAGNodeConnection( const tDAGNodeInputPtr& i, const tDAGNodeOutputPtr& o, const wxString& text = "", f32 textPos = 30.f );
		~tDAGNodeConnection( );

		void fSetText( const std::string& text ) { mText = text; }
		const std::string& fText( ) const { return mText; }
		void fSetTextPosition( f32 position ) { mTextPosition = position; }
		f32 fTextPosition( ) const { return mTextPosition; }
		void fSetColor( const wxColor& color ) { mColor = color; }

		void fUpdate( const wxPoint& currentPos );
		void fOnPaint( const wxPoint& topLeftOrigin, const wxSize& clipDims, b32 curved, wxDC& dc );
		void fRehook( );
		void fUnhook( );
		void fRemoveInput( );
		b32 fComplete( const tDAGNodeInputPtr& input, b32 preventCycles );
		b32 fComplete( const tDAGNodeOutputPtr& output, b32 preventCycles );
		const tDAGNodeInputPtr& fInput( ) const { return mInput; }
		const tDAGNodeOutputPtr& fOutput( ) const { return mOutput; }
		b32 fSelfConnected( ) const { return ( mInput && mOutput && &mInput->fOwner( ) == &mOutput->fOwner( ) ); }

		b32 fIntersects( const wxRect& rect );

		static const wxColor gDefaultConnectionColor;
	};

}

#endif//__tDAGNode__
