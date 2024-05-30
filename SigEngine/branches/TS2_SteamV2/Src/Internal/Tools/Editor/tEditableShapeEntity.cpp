#include "ToolsPch.hpp"
#include "tEditableShapeEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tShapeEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"

namespace Sig { namespace
{
	const f32 gDefaultRadius = 1.0f; // can never change this without breaking all existing dummy objects
	static const char* fEditablePropStateMask( ) { return "States.StateMask"; }
	static const char* fEditablePropShapeType( ) { return "Shape.VolumeType"; }
	static const u32 fStateMaskInitialValue( ) { return ~0; }

	// These will be removed when loaded. It will silently integrate itself into the file if the user chooses to save.
	static const char* gDeprecatedProperties[ ] = { "States.StateIndex" };
}}

namespace Sig { namespace Sigml
{
	class tools_export tShapeObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tShapeObject, 0x412A9E1E );
	public:
		tShapeObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tShapeObject& o )
	{
	}

	register_rtti_factory( tShapeObject, false );

	tShapeObject::tShapeObject( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
	}

	void tShapeObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tShapeObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );

		for( u32 i = 0; i < array_length( gDeprecatedProperties ); ++i )
			mEditableProperties.fRemove( gDeprecatedProperties[ i ] );
	}

	tEntityDef* tShapeObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tShapeEntityDef* entityDef = new tShapeEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		const u32 stateMask = mEditableProperties.fGetValue( fEditablePropStateMask( ), fStateMaskInitialValue( ) );
		entityDef->mStateMask = stateMask & tStateableEntity::cMaxStateMaskValue;
		entityDef->mBounds = Math::tAabbf( Math::tVec3f( -gDefaultRadius ), Math::tVec3f( +gDefaultRadius ) );
		entityDef->mShapeType = ( tShapeEntityDef::tShapeType )mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		return entityDef;
	}

	tEditableObject* tShapeObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableShapeEntity( container, *this );
	}

}}




namespace Sig
{
	namespace
	{
		static const Math::tVec4f cDummyTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.5f );
	}

	tEditableShapeEntity::tEditableShapeEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableShapeEntity::tEditableShapeEntity( tEditableObjectContainer& container, const Sigml::tShapeObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditableShapeEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyMask( fEditablePropStateMask( ), fStateMaskInitialValue( ) ) ) );

		sig_static_assert( tShapeEntityDef::cShapeTypeCount == 2 );
		tDynamicArray<std::string> shapeTypes( tShapeEntityDef::cShapeTypeCount );
		shapeTypes[ tShapeEntityDef::cShapeTypeBox		] = "Box";
		shapeTypes[ tShapeEntityDef::cShapeTypeSphere	] = "Sphere";
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( fEditablePropShapeType( ), shapeTypes, tShapeEntityDef::cShapeTypeBox ) ) );

	}

	void tEditableShapeEntity::fCommonCtor( )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gDefaultRadius;
		localSpaceBox.mMax *= gDefaultRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		const Math::tMat3f xform( gDefaultRadius );
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );

		// setup the box object
		mDummyBox->fSetRgbaTint( cDummyTint );
		mDummyBox->fSetParentRelativeXform( xform );
		if( shapeType == tShapeEntityDef::cShapeTypeBox )
			mDummyBox->fSetInvisible( false );

		// setup the sphere object
		mDummySphere->fSetRgbaTint( cDummyTint );
		mDummySphere->fSetParentRelativeXform( xform );
		if( shapeType == tShapeEntityDef::cShapeTypeSphere )
			mDummySphere->fSetInvisible( false );

		tEditableObject::fUpdateStateTint( );
	}

	tEditableShapeEntity::~tEditableShapeEntity( )
	{
	}

	std::string tEditableShapeEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Shape - " + name;
		return "Shape";
	}

	b32 tEditableShapeEntity::fUniformScaleOnly( )
	{
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		return( shapeType == tShapeEntityDef::cShapeTypeSphere );
	}

	void tEditableShapeEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( &entity == mDummyBox.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyBox->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyBox->fSetRgbaTint( cDummyTint );
		}
		if( &entity == mDummySphere.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummySphere->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummySphere->fSetRgbaTint( cDummyTint );
		}
	}

	Sigml::tObjectPtr tEditableShapeEntity::fSerialize( b32 clone ) const
	{
		Sigml::tShapeObject* ao = new Sigml::tShapeObject( );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	void tEditableShapeEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName( ) == fEditablePropShapeType( ) )
		{
			const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
			if( shapeType == tShapeEntityDef::cShapeTypeBox )
			{
				mDummyBox->fSetInvisible( false );
				mDummySphere->fSetInvisible( true );
			}
			else if( shapeType == tShapeEntityDef::cShapeTypeSphere )
			{
				mDummyBox->fSetInvisible( true );
				mDummySphere->fSetInvisible( false );
			}
		}
	}

}

