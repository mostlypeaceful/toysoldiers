#ifndef __DerivedAnimNodes__
#define __DerivedAnimNodes__

#include "tDAGNode.hpp"
#include "Editor/tEditableProperty.hpp"

namespace Sig
{

	class tXmlSerializer;
	class tXmlDeserializer;

	class tAnimBaseNode;
	typedef tRefCounterPtr< tAnimBaseNode > tAnimBaseNodePtr;

	class tools_export tAnimBaseNode : public Rtti::tSerializableBaseClass, public tDAGNode
	{
		declare_null_reflector();
	public:
		static const std::string cNamePropertiesName;
		static const wxColor cDefaultInputColor;
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

		mutable tEditablePropertyTable mProps;
	public:
		explicit tAnimBaseNode( const std::string& typeName, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft = wxPoint( 0, 0 ) );

		void fSetTypeName( const std::string& name ) { mTypeName = name; }
		const std::string& fTypeName( ) const { return mTypeName; }

		virtual wxString fDisplayName( ) const;
		virtual void fSerializeDerived( tXmlSerializer& s ) { }
		virtual void fSerializeDerived( tXmlDeserializer& s ) { }
		virtual void fApplyPropertyValues( ) { }

		u32 fVersion( ) const { return mVersion; }
		u32 fLoadedVersion( ) const { return mLoadedVersion; }
		void fSetLoadedVersion( u32 version ) { mLoadedVersion = version; }

		tEditablePropertyTable& fProps( ) { return mProps; }
		const tEditablePropertyTable& fProps( ) const { return mProps; }

		tDAGNodeInput* fAddInput( const std::string& inputName, tDAGNodeOutput::tEdge edge = tDAGNodeOutput::cLeft, const wxColor& inputColor = cDefaultInputColor, const wxString& toolTip = "" );
		void fAddOutput( const tDAGNodeOutputPtr& output, u32 mIndex );
		void fRemoveOutput( const tDAGNodeOutputPtr& output );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tAnimBaseNode& o )
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

		tEditablePropertyTable savedProps = o.fProps( );
		s( "Props", o.fProps( ) );
		if( s.fIn( ) )
			o.fProps( ).fUnion( savedProps, false );

		o.fSerializeDerived( s );
	}

	class tools_export tAnimInputNode : public tAnimBaseNode
	{
	public:
		implement_rtti_serializable_base_class( tAnimInputNode, 0xC7745EE9 );

		explicit tAnimInputNode( const wxPoint& p = wxPoint( 0, 0 ) );

		std::string fAnimName( ) const;
		f32 fTimeScale( ) const;

		virtual void fApplyPropertyValues( );

	private:

	};

	class tools_export tAnimBlendNode : public tAnimBaseNode
	{
	public:
		implement_rtti_serializable_base_class( tAnimBlendNode, 0x1471A7F7 );

		explicit tAnimBlendNode( const wxPoint& p = wxPoint( 0, 0 ) );

		std::string fBlendName( ) const;
		b32 fBehaviorDigital( ) const;
		f32 fDigitalThreshold( ) const; // threshold at which a digital blend will trigger from an analog input
		b32 fBehaviorOneShot( ) const;

		f32 fACurve( ) const;
		f32 fBCurve( ) const;

		f32 fTimeScale( ) const;

		// True if the UI should display a check box instead of a slider.
		b32 fUIBehaviorDigital( ) const;
		b32 fUIOnlyLinkTimeScale( ) const;

		virtual void fApplyPropertyValues( );

	private:

	};

	class tools_export tAnimStackNode : public tAnimBaseNode
	{
	public:
		implement_rtti_serializable_base_class( tAnimStackNode, 0xEFFAB946 );

		explicit tAnimStackNode( const wxPoint& p = wxPoint( 0, 0 ) );

		void fRefreshInputNames( );
		void fCheckConnections( );

	private:
		virtual void fSerializeDerived( tXmlSerializer& s );
		virtual void fSerializeDerived( tXmlDeserializer& s );

	};

	class tools_export tAnimOutputNode : public tAnimStackNode
	{
	public:
		implement_rtti_serializable_base_class( tAnimOutputNode, 0x218EFB19 );

		explicit tAnimOutputNode( const wxPoint& p = wxPoint( 0, 0 ) );

	private:

	};
}


#endif//__DerivedAnimNodes__
