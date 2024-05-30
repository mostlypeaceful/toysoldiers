#include "ToolsPch.hpp"
#include "DerivedAnimNodes.hpp"
#include "tDAGNodeCanvas.hpp"

#include "Editor/tEditablePropertyTypes.hpp"


namespace Sig
{

	namespace
	{
		const wxColour cColorValuesBarColorAnim ( 0xcc, 0xa3, 0x00 );
		const wxColour cColorValuesTextColorAnim( 0x00, 0x00, 0x00 );

		const wxColour cColorValuesBarColorBlend ( 0xaa, 0x11, 0x88 ); //0x22, 0x19, 0xb2 );
		const wxColour cColorValuesTextColorBlend( 0xff, 0xff, 0x99 );

		const wxColour cColorValuesOutput( 0x99, 0x99, 0x99 );
		const wxColour cColorValuesInput( 0x99, 0x99, 0x99 );
	}

	u32 tAnimBaseNode::gNextUniqueNodeId = 0;	
	const wxColor tAnimBaseNode::cDefaultInputColor = wxColour( 0x11, 0xee, 0x11 );

	void tAnimBaseNode::fSetNextUniqueNodeId( u32 nextUid )
	{
		gNextUniqueNodeId = nextUid;
	}

	void tAnimBaseNode::fGrabNextUniqueNodeId( )
	{
		mUniqueNodeId = gNextUniqueNodeId++;
	}


	tAnimBaseNode::tAnimBaseNode( const std::string& typeName, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft )
		: tDAGNode( typeName, titleBarColor, titleTextColor, topLeft )
		, mVersion( 1 )
		, mLoadedVersion( mVersion )
		, mUniqueNodeId( gNextUniqueNodeId++ )
		, mTypeName( typeName )
	{
	}

	wxString tAnimBaseNode::fDisplayName( ) const
	{
		return tDAGNode::fName( );
	}

	tDAGNodeInput* tAnimBaseNode::fAddInput( const std::string& inputName, tDAGNodeOutput::tEdge edge, const wxColor& inputColor, const wxString& toolTip )
	{
		tDAGNodeInput *newIO = new tDAGNodeInput( 
			inputName,
			*this, 
			inputColor,
			1,
			edge );

		newIO->fSetToolTip( toolTip );
		mInputs.fPushBack( tDAGNodeInputPtr( newIO ) );
		return newIO;
	}
	void tAnimBaseNode::fAddOutput( const tDAGNodeOutputPtr& output, u32 mIndex )
	{
		u32 index = mOutputs.fCount( );
		mOutputs.fInsertSafe( mIndex, output );
	}
	void tAnimBaseNode::fRemoveOutput( const tDAGNodeOutputPtr& output )
	{
		mOutputs.fFindAndEraseOrdered( output );
	}


	///////////////////////////////////////////////////////////////////////////////////////////////


	namespace
	{
		static const char* cAnimNamePropertyName = "Anim.Name";
		static const char* cAnimTimeScalePropertyName = "Anim.TimeScale";

		static const f32 cEditableFloatMin = -9999.f;
		static const f32 cEditableFloatMax = +9999.f;
	}
	
	register_rtti_factory( tAnimInputNode, false );	

	tAnimInputNode::tAnimInputNode( const wxPoint& p )
		: tAnimBaseNode( "Anim", cColorValuesBarColorAnim, cColorValuesTextColorAnim, false, p )
	{
		fAddOutput( tDAGNodeOutputPtr( new tDAGNodeOutput( "Anim", *this, cColorValuesOutput ) ), 0 );
		mTexts.fPushBack( tDAGNodeTextPtr( new tDAGNodeText( "", "", *this, true ) ) );

		fComputeDimensions( );

		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyString( cAnimNamePropertyName, false, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cAnimTimeScalePropertyName, 1.f, cEditableFloatMin, cEditableFloatMax, 0.1f, 2, false ) ) );
	}

	std::string tAnimInputNode::fAnimName( ) const
	{
		return mProps.fGetValue<std::string>( cAnimNamePropertyName, "" );
	}

	f32 tAnimInputNode::fTimeScale( ) const
	{
		return mProps.fGetValue<f32>( cAnimTimeScalePropertyName, 1.f );
	}

	void tAnimInputNode::fApplyPropertyValues( )
	{ 
		mTexts[ 0 ]->fSetValue( fAnimName( ) );
		fComputeDimensions( );
	}


	///////////////////////////////////////////////////////////////////////////////////////////////


	namespace
	{
		static const char* cBlendNamePropertyName = "Blend.Name";
		static const char* cDigitalPropertyName = "Behavior.Digital";
		static const char* cDigitalThresholdPropertyName = "Behavior.Digital Threshold";
		static const char* cOneShotPropertyName = "Behavior.OneShot";

		static const char* cCurveAPropertyName = "Curve.A";
		static const char* cCurveBPropertyName = "Curve.B";

		// these properties dont make it to the game
		static const char* cUIOnlyLinkTimeScalePropertyName = "UI Only.TimeScale";
	}

	register_rtti_factory( tAnimBlendNode, false );	

	tAnimBlendNode::tAnimBlendNode( const wxPoint& p )
		: tAnimBaseNode( "Blend", cColorValuesBarColorBlend, cColorValuesTextColorBlend, false, p )
	{
		fAddInput( "A", tDAGNodeOutput::cLeft, cColorValuesInput );
		fAddInput( "B", tDAGNodeOutput::cLeft, cColorValuesInput );

		fAddOutput( tDAGNodeOutputPtr( new tDAGNodeOutput( "Result", *this, cColorValuesOutput ) ), 0 );

		mTexts.fPushBack( tDAGNodeTextPtr( new tDAGNodeText( "", "", *this, true ) ) );

		fComputeDimensions( );

		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyString( cBlendNamePropertyName, false, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cDigitalPropertyName, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cDigitalThresholdPropertyName, 0.f, cEditableFloatMin, cEditableFloatMax, 0.1f, 1, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cOneShotPropertyName, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cCurveAPropertyName, 0.f, cEditableFloatMin, cEditableFloatMax, 0.1f, 1, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cCurveBPropertyName, 1.f, cEditableFloatMin, cEditableFloatMax, 0.1f, 1, false ) ) );
		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cAnimTimeScalePropertyName, 1.f, cEditableFloatMin, cEditableFloatMax, 0.1f, 2, false ) ) );

		mProps.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cUIOnlyLinkTimeScalePropertyName, false ) ) );
	}

	std::string tAnimBlendNode::fBlendName( ) const
	{
		return mProps.fGetValue<std::string>( cBlendNamePropertyName, "" );
	}

	b32 tAnimBlendNode::fBehaviorDigital( ) const
	{
		return mProps.fGetValue<b32>( cDigitalPropertyName, false );
	}

	f32 tAnimBlendNode::fDigitalThreshold( ) const
	{
		return mProps.fGetValue<f32>( cDigitalThresholdPropertyName, 0.f );
	}

	b32 tAnimBlendNode::fBehaviorOneShot( ) const
	{
		return mProps.fGetValue<b32>( cOneShotPropertyName, false );
	}

	f32 tAnimBlendNode::fACurve( ) const
	{
		return mProps.fGetValue<f32>( cCurveAPropertyName, 0.f );
	}

	f32 tAnimBlendNode::fBCurve( ) const
	{
		return mProps.fGetValue<f32>( cCurveBPropertyName, 1.f );
	}

	f32 tAnimBlendNode::fTimeScale( ) const
	{
		return mProps.fGetValue<f32>( cAnimTimeScalePropertyName, 1.f );
	}

	b32 tAnimBlendNode::fUIBehaviorDigital( ) const
	{
		return (fBehaviorDigital( ) && fDigitalThreshold( ) == 0.f);
	}

	b32 tAnimBlendNode::fUIOnlyLinkTimeScale( ) const
	{
		return mProps.fGetValue<b32>( cUIOnlyLinkTimeScalePropertyName, false );
	}

	void tAnimBlendNode::fApplyPropertyValues( )
	{ 
		mTexts[ 0 ]->fSetValue( fBlendName( ) );
		fComputeDimensions( );
	}


	///////////////////////////////////////////////////////////////////////////////////////////////

	register_rtti_factory( tAnimStackNode, false );	

	tAnimStackNode::tAnimStackNode( const wxPoint& p )
		: tAnimBaseNode( "Combine", cColorValuesBarColorBlend, cColorValuesTextColorBlend, false, p )
	{
		fAddOutput( tDAGNodeOutputPtr( new tDAGNodeOutput( "Result", *this, cColorValuesOutput ) ), 0 );

		fCheckConnections( );
	}

	void tAnimStackNode::fSerializeDerived( tXmlSerializer& s ) 
	{ 
		u32 inputCount = mInputs.fCount( );
		s( "ic", inputCount );
	}

	void tAnimStackNode::fSerializeDerived( tXmlDeserializer& s ) 
	{ 
		u32 inputCount = 0;
		s( "ic", inputCount );

		mInputs.fSetCount( 0 );
		for( u32 i = 0; i < inputCount; ++i )
			fAddInput( "" );

		fRefreshInputNames( );
		fComputeDimensions( );
	}


	void tAnimStackNode::fRefreshInputNames( )
	{
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
			mInputs[ i ]->fSetName( StringUtil::fToString( i ) );
	}

	void tAnimStackNode::fCheckConnections( )
	{
		//make sure we have one open on the end and top.
		if( mInputs.fCount( ) == 0 )
		{
			// no connections add one.
			fAddInput( "" );
		}

		u32 firstWithConnect = ~0;
		u32 lastWithConnect = ~0;
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( firstWithConnect == ~0 && mInputs[ i ]->fConnectionList( ).fCount( ) )
				firstWithConnect = i;
			else if( firstWithConnect != ~0 && mInputs[ i ]->fConnectionList( ).fCount( ) )
				lastWithConnect = i;
		}

		if( firstWithConnect != ~0 )
		{
			// trim inputs from the front, dont remove first
			for( s32 i = firstWithConnect - 1; i > 0; --i )
			{
				mInputs.fEraseOrdered( i );
				--lastWithConnect;
			}
			// trim inputs from the end, dont remove last
			for( s32 i = mInputs.fCount( ) - 1; i > (s32)lastWithConnect + 1 ; --i )
			{
				mInputs.fEraseOrdered( i );
			}
		}

		// Add new connections if necessary
		if( mInputs.fFront( )->fConnectionList( ).fCount( ) )
		{
			mInputs.fInsert( 0, tDAGNodeInputPtr( new tDAGNodeInput( "", *this, cDefaultInputColor, 1 ) ) );
		}

		if( mInputs.fBack( )->fConnectionList( ).fCount( ) )
		{
			mInputs.fPushBack( tDAGNodeInputPtr( new tDAGNodeInput( "", *this, cDefaultInputColor, 1 ) ) );
		}

		fRefreshInputNames( );
		fComputeDimensions( );
	}


	///////////////////////////////////////////////////////////////////////////////////////////////

	register_rtti_factory( tAnimOutputNode, false );	

	tAnimOutputNode::tAnimOutputNode( const wxPoint& p )
		: tAnimStackNode( p )
	{
		fSetName( "Output" );
		mOutputs.fSetCount( 0 );
		fComputeDimensions( );
	}

}

