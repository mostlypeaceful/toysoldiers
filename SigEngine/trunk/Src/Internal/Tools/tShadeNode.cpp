#include "ToolsPch.hpp"
#include "tShadeNode.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "HlslGen/tHlslWriter.hpp"

namespace Sig
{
	const char tShadeNode::cNamePropertiesName[]="Material Common.DisplayName";
	const char tShadeNode::cNamePropertiesEditable[]="Material Common.Editable";
	const char tShadeNode::cNamePropertiesDefault[]="Material Properties.Default";
	const char tShadeNode::cNamePropertiesMin[]="Material Properties.Min";
	const char tShadeNode::cNamePropertiesMax[]="Material Properties.Max";

	u32 tShadeNode::gNextUniqueNodeId = 1;

	void tShadeNode::fSetNextUniqueNodeId( u32 nextUid )
	{
		gNextUniqueNodeId = nextUid;
	}

	tShadeNode::tShadeNode( const std::string& titleText, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft )
		: tDAGNode( titleText, titleBarColor, titleTextColor, topLeft )
		, mVersion( 1 )
		, mLoadedVersion( mVersion )
		, mUniqueNodeId( gNextUniqueNodeId++ )
		, mMaterialGlueIndex( -1 )
	{
		if( editable )
		{
			mShadeProps.fInsert( tEditablePropertyPtr( new tEditablePropertyString( cNamePropertiesName ) ) );
			fShadeProps( ).fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cNamePropertiesEditable, true ) ) );
		}
	}
	wxString tShadeNode::fDisplayName( ) const
	{
		if( GetAsyncKeyState( VK_SPACE ) & 0x8000 )
		{
			std::stringstream ss;
			ss << fName( ).c_str( ) << ':' << fUniqueNodeId( );
			return ss.str( );
		}

		return fName( );
	}
	b32 tShadeNode::fInputNeedsWritingToHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tHlslGenTree& hlslGenTree, u32 ithInput )
	{
		using namespace HlslGen;
		if( writer.fWriteMode( ) == cWriteModeColor )
			return true;
		if( writer.fWriteMode( ) == cWriteModeDepth )
			return false;
		if( writer.fWriteMode( ) == cWriteModeDepthWithAlpha )
		{
			tHlslGenTreePtr input = hlslGenTree.fInputs( )[ ithInput ];
			if( input )
			{
				if( input->fInputs( ).fCount( ) == 0 )
					return true;
				for( u32 i = 0; i < input->fInputs( ).fCount( ); ++i )
				{
					if( input->fInputs( )[ i ] && input->fShadeNode( )->fInputNeedsWritingToHlsl( writer, *input, i ) )
						return true;
				}
			}
		}
		return false;
	}
	void tShadeNode::fGrabNextUniqueNodeId( )
	{
		mUniqueNodeId = gNextUniqueNodeId++;
	}
	std::string tShadeNode::fMatEdName( ) const
	{
		std::string o="";
		tEditablePropertyPtr* find = mShadeProps.fFind( cNamePropertiesName );
		if( find && *find )
			(*find)->fGetData( o );
		return o;
	}
	std::string tShadeNode::fMatEdDisplayName( s32 index ) const
	{
		std::stringstream ss;

		if( index >= 0 )
			ss << index << ". ";

		const std::string matEdName = fMatEdName( );
		if( matEdName.length( ) > 0 )
			ss << matEdName << " - " << fName( );
		else
			ss << fName( );

		const std::string title = ss.str( );
		return title;
	}
	b32 tShadeNode::fMatEdAllowEdit( ) const
	{
		b32 o = false;
		tEditablePropertyPtr* find = mShadeProps.fFind( cNamePropertiesEditable );
		if( find && *find )
			(*find)->fGetData( o );
		return o;
	}
	void tShadeNode::fAddInput( const std::string& inputName, const wxColor& inputColor )
	{
		mInputs.fPushBack( tDAGNodeInputPtr( new tDAGNodeInput( 
			inputName,
			*this, 
			inputColor,
			1 ) ) );
	}
	void tShadeNode::fAddOutput( const std::string& outputName, const wxColor& outputColor )
	{
		mOutputs.fPushBack( tDAGNodeOutputPtr( new tDAGNodeOutput( 
			outputName, 
			*this, 
			outputColor ) ) );
	}
}

