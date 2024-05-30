#ifndef __tShadeNode__
#define __tShadeNode__
#include "tDAGNode.hpp"
#include "Editor/tEditableProperty.hpp"
#include "HlslGen/tHlslVariable.hpp"

namespace Sig
{
	class tXmlSerializer;
	class tXmlDeserializer;

	namespace HlslGen { class tHlslGenTree; class tHlslWriter; }
	namespace Gfx { class tShadeMaterialGlueValues; }

	class tShadeNode;
	typedef tRefCounterPtr< tShadeNode > tShadeNodePtr;

	class tools_export tShadeNode : public Rtti::tSerializableBaseClass, public tDAGNode
	{
		declare_null_reflector();
		//implement_rtti_serializable_base_class( tShadeNode, 0x6DCC9684 );
	public:
		enum tOutputSemantic
		{
			cOutputColor0,
			cOutputColor1,
			cOutputColor2,
			cOutputColor3,
			cOutputDepth,
			cOutputCount,
			cOutputNotAnOutput = 0xffff,
		};
		typedef tFixedArray<tShadeNodePtr,tShadeNode::cOutputCount> tOutputArray;

	public:
		static const char cNamePropertiesName[];
		static const char cNamePropertiesEditable[];
		static const char cNamePropertiesDefault[];
		static const char cNamePropertiesMin[];
		static const char cNamePropertiesMax[];
	private:
		static u32 gNextUniqueNodeId;
	public:
		static u32 fNextUniqueNodeId( ) { return gNextUniqueNodeId; }
		static void fSetNextUniqueNodeId( u32 nextUid );
	private:
		u32 mVersion, mLoadedVersion;
		u32 mUniqueNodeId;
		s32 mMaterialGlueIndex;
		mutable tEditablePropertyTable mShadeProps, mMatProps;
	public:
		tShadeNode( const wxString& titleText, const wxColour& titleBarColor, const wxColour& titleTextColor, b32 editable, const wxPoint& topLeft = wxPoint( 0, 0 ) );
		virtual wxString fDisplayName( ) const;
		virtual void fSerialize( tXmlSerializer& s ) = 0;
		virtual void fSerialize( tXmlDeserializer& s ) = 0;
		virtual tOutputSemantic fOutputSemantic( ) const { return cOutputNotAnOutput; }
		virtual b32 fRefreshMaterialProperties( tShadeNode& src, u32 index ) { return false; }
		virtual b32 fInputNeedsWritingToHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tHlslGenTree& hlslGenTree, u32 ithInput );
		virtual void fAddHlslRequirements( HlslGen::tHlslWriter& writer, HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree ) { }
		virtual void fWriteHlsl( HlslGen::tHlslWriter& writer, const HlslGen::tShaderRequirements& reqs, HlslGen::tHlslGenTree& hlslGenTree ) { }
		virtual void fUpdateMaterialGlueValues( Gfx::tShadeMaterialGlueValues& glueVals, Dx9Util::tTextureCache& textureCache ) { }
		virtual void fAcquireMaterialGlueValuesForAssetGen( Gfx::tShadeMaterialGlueValues& glueVals, tLoadInPlaceFileBase& lipFileCreator, tFilePathPtrList& resourcePathsOut ) { }
		u32 fVersion( ) const { return mVersion; }
		u32 fLoadedVersion( ) const { return mLoadedVersion; }
		void fSetLoadedVersion( u32 version ) { mLoadedVersion = version; }
		u32 fUniqueNodeId( ) const { return mUniqueNodeId; }
		void fSetUniqueNodeId( u32 uid ) { mUniqueNodeId = uid; } // be careful, you probably shouldn't be calling this
		void fGrabNextUniqueNodeId( );
		s32 fMaterialGlueIndex( ) const { return mMaterialGlueIndex; }
		void fSetMaterialGlueIndex( s32 newIndex = -1 ) { mMaterialGlueIndex = newIndex; }
		std::string fMatEdName( ) const;
		std::string fMatEdDisplayName( s32 index = -1 ) const;
		b32 fMatEdAllowEdit( ) const;
		tEditablePropertyTable& fShadeProps( ) { return mShadeProps; }
		const tEditablePropertyTable& fShadeProps( ) const { return mShadeProps; }
		tEditablePropertyTable& fMatProps( ) { return mMatProps; }
		const tEditablePropertyTable& fMatProps( ) const { return mMatProps; }
		void fAddInput( const wxString& inputName, const wxColor& inputColor = wxColour( 0x11, 0xee, 0x11 ) );
		void fAddOutput( const wxString& outputName = "", const wxColor& outputColor = wxColour( 0xff, 0xff, 0xff ) );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tShadeNode& o )
	{
		u32 version = o.fVersion( );
		s( "Version", version );
		o.fSetLoadedVersion( version );

		u32 uid = o.fUniqueNodeId( );
		s( "UniqueNodeId", uid );
		o.fSetUniqueNodeId( uid );

		s32 mgi = o.fMaterialGlueIndex( );
		s( "MaterialGlueIndex", mgi );
		o.fSetMaterialGlueIndex( mgi );

		Math::tVec2i topLeft( o.fTopLeft( ).x, o.fTopLeft( ).y );
		s( "TopLeft", topLeft );
		o.fSetPosition( wxPoint( topLeft.x, topLeft.y ) );


		tEditablePropertyTable savedShadeProps = o.fShadeProps( );
		s( "ShadeProps", o.fShadeProps( ) );
		if( s.fIn( ) )
			o.fShadeProps( ).fUnion( savedShadeProps, false );

		s( "MatProps", o.fMatProps( ) );

		o.fSerialize( s );
	}

#define implement_derived_shade_node( typeName, typeId ) \
	implement_rtti_serializable_base_class( typeName, typeId ); \
	virtual void fSerialize( tXmlSerializer& s ) { fSerializeGeneric( s ); } \
	virtual void fSerialize( tXmlDeserializer& s ) { fSerializeGeneric( s ); } \
	public:\
	template<class tSerializer> \
	void fSerializeGeneric( tSerializer& s )

}

#endif//__tShadeNode__
