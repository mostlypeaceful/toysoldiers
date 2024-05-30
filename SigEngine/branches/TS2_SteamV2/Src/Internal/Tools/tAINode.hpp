#ifndef __tAINode__
#define __tAINode__
#include "tDAGNode.hpp"
#include "Editor/tEditableProperty.hpp"

namespace Sig
{
	class tXmlSerializer;
	class tXmlDeserializer;

	class tAINodeControlPanel
	{
	public:
		virtual ~tAINodeControlPanel( ) { }

		virtual void fRefreshProperties( ) { }
	};

	class tAINode;
	typedef tRefCounterPtr< tAINode > tAINodePtr;

	class tools_export tAINode : public Rtti::tSerializableBaseClass, public tDAGNode
	{
		declare_null_reflector();
	public:
		static const std::string cNamePropertiesName;
	private:
		static u32 gNextUniqueNodeId;
	public:
		static u32 fNextUniqueNodeId( ) { return gNextUniqueNodeId; }
		static void fSetNextUniqueNodeId( u32 nextUid );
		void fGrabNextUniqueNodeId( );

		u32 fUniqueNodeId( ) const { return mUniqueNodeId; }
		void fSetUniqueNodeId( u32 uid ) { mUniqueNodeId = uid; } // be careful, you probably shouldn't be calling this

	protected:
		u32 mVersion, mLoadedVersion;
		u32 mUniqueNodeId;
		std::string mTypeName;

		mutable tEditablePropertyTable mAIProps;
		tAINodeControlPanel* mControlPanel;
	public:
		tAINode( const wxString& typeName, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft = wxPoint( 0, 0 ) );

		void fSetTypeName( const std::string& name ) { mTypeName = name; }
		const std::string& fTypeName( ) const { return mTypeName; }

		virtual wxString fDisplayName( ) const;
		virtual void fSerialize( tXmlSerializer& s ) = 0;
		virtual void fSerialize( tXmlDeserializer& s ) = 0;
		virtual void fApplyPropertyValues( ) { }

		u32 fVersion( ) const { return mVersion; }
		u32 fLoadedVersion( ) const { return mLoadedVersion; }
		void fSetLoadedVersion( u32 version ) { mLoadedVersion = version; }

		tEditablePropertyTable& fAIProps( ) { return mAIProps; }
		const tEditablePropertyTable& fAIProps( ) const { return mAIProps; }
		void fSetControlPanel( tAINodeControlPanel* panel ) { mControlPanel = panel; }

		tDAGNodeInput* fAddInput( const wxString& inputName, tDAGNodeOutput::tEdge edge = tDAGNodeOutput::cLeft, const wxColor& inputColor = wxColour( 0x11, 0xee, 0x11 ), const wxString& toolTip = "" );
		void fAddOutput( const tDAGNodeOutputPtr& output, u32 mIndex );
		void fRemoveOutput( const tDAGNodeOutputPtr& output );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tAINode& o )
	{
		u32 version = o.fVersion( );
		s( "Version", version );
		o.fSetLoadedVersion( version );

		u32 uid = o.fUniqueNodeId( );
		s( "UniqueNodeId", uid );
		o.fSetUniqueNodeId( uid );

		Math::tVec2i topLeft( o.fTopLeft( ).x, o.fTopLeft( ).y );
		s( "TopLeft", topLeft );
		o.fSetPosition( wxPoint( topLeft.x, topLeft.y ) );

		tEditablePropertyTable savedAIProps = o.fAIProps( );
		s( "AIProps", o.fAIProps( ) );
		if( s.fIn( ) )
			o.fAIProps( ).fUnion( savedAIProps, false );

		o.fSerialize( s );
	}

#define implement_derived_ai_node( typeName, typeId ) \
	implement_rtti_serializable_base_class( typeName, typeId );

}

#endif//__tAINode__
