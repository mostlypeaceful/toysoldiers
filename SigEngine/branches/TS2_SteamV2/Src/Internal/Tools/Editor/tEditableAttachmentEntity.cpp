#include "ToolsPch.hpp"
#include "tEditableAttachmentEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tAttachmentEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"

namespace Sig { namespace
{
	const f32 gSmallRadius = 0.1f;
	const f32 gBigRadius = 1.f;

	static const char* fEditablePropStateMask( ) { return "States.StateMask"; }
	static const u32 fStateMaskInitialValue( ) { return ~0; }
}}

namespace Sig { namespace Sigml
{
	class tools_export tAttachmentObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tAttachmentObject, 0x4E6BA2CF );
	public:

	public:
		tAttachmentObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tAttachmentObject& o )
	{
	}

	register_rtti_factory( tAttachmentObject, false );

	tAttachmentObject::tAttachmentObject( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
	}

	void tAttachmentObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tAttachmentObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}

	tEntityDef* tAttachmentObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		Math::tMat3f saveXform = mXform;
		mXform.fNormalizeBasis( );

		tAttachmentEntityDef* entityDef = new tAttachmentEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );
		entityDef->mBounds = Math::tAabbf( Math::tVec3f( -gSmallRadius ), Math::tVec3f( +gSmallRadius ) );
		
		const u32 stateMask = mEditableProperties.fGetValue( fEditablePropStateMask( ), fStateMaskInitialValue( ) );
		entityDef->mStateMask = stateMask & tStateableEntity::cMaxStateMaskValue;

		mXform = saveXform;

		return entityDef;
	}

	tEditableObject* tAttachmentObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableAttachmentEntity( container, *this );
	}
}}




namespace Sig
{
	namespace
	{
		static const Math::tVec4f gInnerBallTint = Math::tVec4f( 0.1f, 0.3f, 1.0f, 0.75f );
		static const Math::tVec4f gShellSphereTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.25f );
	}

	tEditableAttachmentEntity::tEditableAttachmentEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fCommonCtor( );
	}

	tEditableAttachmentEntity::tEditableAttachmentEntity( tEditableObjectContainer& container, const Sigml::tAttachmentObject& ao )
		: tEditableObject( container )
	{
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditableAttachmentEntity::fCommonCtor( )
	{
		mShellRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mShellRenderState.fEnableDisable( Gfx::tRenderState::cFillWireFrame, true );

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gBigRadius;
		localSpaceBox.mMax *= gBigRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		// setup the smaller center
		mDummySphere->fSetRgbaTint( gInnerBallTint );
		const Math::tMat3f mSmall( gSmallRadius );
		mDummySphere->fSetParentRelativeXform( mSmall );
		mDummySphere->fSetInvisible( false );

		// create the outer shell (larger radius, wireframe)
		mShellSphere.fReset( new tDummyObjectEntity( 
			mContainer.fGetDummySphereTemplate( ).fGetModifiedRenderBatch( &mShellRenderState ), 
			mContainer.fGetDummySphereTemplate( ).fGetBounds( ), true ) );
		mShellSphere->fSetRgbaTint( gShellSphereTint );
		const Math::tMat3f mBig( gBigRadius );
		mShellSphere->fSetParentRelativeXform( mBig );
		mShellSphere->fSpawnImmediate( *this );

		fAddEditableProperties( );

		tEditableObject::fUpdateStateTint( );
	}

	tEditableAttachmentEntity::~tEditableAttachmentEntity( )
	{
	}

	std::string tEditableAttachmentEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Attachment - " + name;
		return "Attachment";
	}

	void tEditableAttachmentEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyMask( fEditablePropStateMask( ), fStateMaskInitialValue( ) ) ) );
	}

	void tEditableAttachmentEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( !fIsFrozen( ) )
		{
			if( &entity == mDummySphere.fGetRawPtr( ) )
				mDummySphere->fSetRgbaTint( gInnerBallTint );
			else if( &entity == mShellSphere.fGetRawPtr( ) )
				mShellSphere->fSetRgbaTint( gShellSphereTint );
		}
	}

	Sigml::tObjectPtr tEditableAttachmentEntity::fSerialize( b32 clone ) const
	{
		Sigml::tAttachmentObject* ao = new Sigml::tAttachmentObject( );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

}

