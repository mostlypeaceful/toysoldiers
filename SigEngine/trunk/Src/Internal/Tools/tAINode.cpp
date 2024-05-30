#include "ToolsPch.hpp"
#include "tAINode.hpp"
#include "DerivedAINodes.hpp"
#include "Editor/tEditablePropertyTypes.hpp"

namespace Sig
{
	const std::string tAINode::cNamePropertiesName = "Goal Properties.Name Override";

	u32 tAINode::gNextUniqueNodeId = 1;

	void tAINode::fSetNextUniqueNodeId( u32 nextUid )
	{
		gNextUniqueNodeId = nextUid;
	}

	void tAINode::fGrabNextUniqueNodeId( )
	{
		mUniqueNodeId = gNextUniqueNodeId++;
	}

	
	tAINode::tAINode( const std::string& typeName, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft )
		: tDAGNode( typeName, titleBarColor, titleTextColor, topLeft )
		, mVersion( 1 )
		, mLoadedVersion( mVersion )
		, mUniqueNodeId( gNextUniqueNodeId++ )
		, mTypeName( typeName )
		, mControlPanel( NULL )
	{
	}

	wxString tAINode::fDisplayName( ) const
	{
		return tDAGNode::fName( );
	}
	tDAGNodeInput* tAINode::fAddInput( const std::string& inputName, tDAGNodeOutput::tEdge edge, const wxColor& inputColor, const wxString& toolTip )
	{
		tDAGNodeInput *newIO = new tDAGNodeInput( 
			inputName,
			*this, 
			inputColor,
			~0,
			edge );

		newIO->fSetToolTip( toolTip );
		mInputs.fPushBack( tDAGNodeInputPtr( newIO ) );
		return newIO;
	}
	void tAINode::fAddOutput( const tDAGNodeOutputPtr& output, u32 mIndex )
	{
		u32 index = mOutputs.fCount( );
		mOutputs.fInsertSafe( mIndex, output );
	}
	void tAINode::fRemoveOutput( const tDAGNodeOutputPtr& output )
	{
		mOutputs.fFindAndEraseOrdered( output );
	}
}

