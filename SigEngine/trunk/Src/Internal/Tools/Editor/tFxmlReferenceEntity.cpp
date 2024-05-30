#include "ToolsPch.hpp"
#include "tSceneRefEntity.hpp"
#include "tFxmlReferenceEntity.hpp"
#include "Fxml.hpp"
#include "tSigmlConverter.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "FX/tFxFile.hpp"

namespace Sig
{
	namespace
	{
		static const char* fEditablePropStateMask( ) { return "States.StateMask"; }
		static const u32 fStateMaskInitialValue( ) { return ~0; }
		static const char* fEditablePropLocalSpace( ) { return "LocalSpace"; }
		static const char* fEditablePropLoopCount( ) { return "LoopCount"; }
		static const char* fEditablePropPreLoadTime( ) { return "PreLoadTime"; }
		static const char* fEditablePropSurfaceOrient( ) { return "SurfaceOrient"; }
		static const char* fEditablePropNeverCull( ) { return "NeverCull"; }
	}
}

namespace Sig { namespace Sigml
{
	///
	/// \brief Object representing a reference to another sigml file.
	class tools_export tFxmlReferenceObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tFxmlReferenceObject, 0xE30FB49A );
	public:
		tFilePathPtr mReferencePath;
	public:
		tFxmlReferenceObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );

		tResourceId& fConstructResourceId( tResourceId& rid ) const;
	};
	typedef tRefCounterPtr< tFxmlReferenceObject > tFxmlReferenceObjectPtr;


	///
	/// \section tFxmlReferenceObject
	///

	register_rtti_factory( tFxmlReferenceObject, false );

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFxmlReferenceObject& o )
	{
		s( "ReferencePath", o.mReferencePath );
	}

	tFxmlReferenceObject::tFxmlReferenceObject( )
	{
	}

	void tFxmlReferenceObject::fSerialize( tXmlSerializer& s )		{ fSerializeXmlObject( s, *this ); }
	void tFxmlReferenceObject::fSerialize( tXmlDeserializer& s )	{ fSerializeXmlObject( s, *this ); }

	void tFxmlReferenceObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );
		resourcePathsOut.fFindOrAdd( mReferencePath );
	}

	tEntityDef* tFxmlReferenceObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tEffectRefEntityDef* entityDef = new tEffectRefEntityDef( );

		fConvertEntityDefBase( entityDef, sigmlConverter );

		tResourceId rid;
		entityDef->mReferenceFile = sigmlConverter.fAddLoadInPlaceResourcePtr( fConstructResourceId( rid ) );

		// fade setting and override
		const u32 fadeSetting = mEditableProperties.fGetValue( fEditablePropFadeSetting( ), 0u );
		const f32 fadeOverride = mEditableProperties.fGetValue( fEditablePropFadeOverride( ), 0.f );
		if( fadeSetting || fadeOverride )
		{
			entityDef->mLODSettings = new tSceneLODSettings;
			entityDef->mLODSettings->mFadeSetting = fadeSetting;
			entityDef->mLODSettings->mFadeOverride = fadeOverride;
		}

		entityDef->mStartupFlags = 0;
		if( mEditableProperties.fGetValue<b32>( fEditablePropLocalSpace( ), false ) )
			entityDef->mStartupFlags |= tEffectRefEntityDef::cStartupFlagLocalSpace;
		if( mEditableProperties.fGetValue<b32>( fEditablePropNeverCull( ), false ) )
			entityDef->mStartupFlags |= tEffectRefEntityDef::cStartupFlagNeverCull;
		
		entityDef->mLoopCount = mEditableProperties.fGetValue<s32>( fEditablePropLoopCount( ), -1 );
		entityDef->mPreLoadTime = mEditableProperties.fGetValue<f32>( fEditablePropPreLoadTime( ), 0.f );
		entityDef->mSurfaceOrientation = mEditableProperties.fGetValue<u32>( fEditablePropSurfaceOrient( ), 0 );

		const u32 stateMask = mEditableProperties.fGetValue( fEditablePropStateMask( ), fStateMaskInitialValue( ) );
		entityDef->mStateMask = stateMask & tStateableEntity::cMaxStateMaskValue;

		return entityDef;
	}

	tEditableObject* tFxmlReferenceObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tFxmlReferenceEntity( container, *this );
	}

	tResourceId& tFxmlReferenceObject::fConstructResourceId( tResourceId& rid ) const
	{
		const tFilePathPtr fxbPath = FX::tFxFile::fFxmlPathToFxb( mReferencePath );
		rid = tResourceId::fMake<FX::tFxFile>( fxbPath );
		return rid;
	}

}}


namespace Sig
{

	tFxmlReferenceEntity::tFxmlReferenceEntity( tEditableObjectContainer& container, const tResourcePtr& fxResource )
		: tEditableObject( container )
	{
		fAddEditableProperties( );

		fCommonCtor( fxResource );
	}

	tFxmlReferenceEntity::tFxmlReferenceEntity( tEditableObjectContainer& container, const Sigml::tFxmlReferenceObject& fxmlObject )
		: tEditableObject( container )
	{
		fAddEditableProperties( );

		fDeserializeBaseObject( &fxmlObject );

		tResourceId rid;
		tResourcePtr fxResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( fxmlObject.fConstructResourceId( rid ) );

		fCommonCtor( fxResource );
	}

	void tFxmlReferenceEntity::fAddEditableProperties( )
	{
		tDynamicArray<std::string> orientNames;
		orientNames.fResize( 4 );
		orientNames[ 0 ] = "Sigml (Unchanged)";
		orientNames[ 1 ] = "Y Up";
		orientNames[ 2 ] = "Surface Normal";
		orientNames[ 3 ] = "Input Normal";

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyMask( fEditablePropStateMask( ), fStateMaskInitialValue( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropBoneRelativeAttachment( ), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropLocalSpace( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropLoopCount( ), -1.f, -1.f, 9999999.f, 1.f, 0, false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropPreLoadTime( ), 0.f, 0.f, 9999999.f, 0.01f, 2, false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( fEditablePropSurfaceOrient( ), orientNames, 0, false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropNeverCull( ) ) ) );
		Sigml::tObjectProperties::fAddFadeSettingsEditableProperties( mEditableProperties );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCDCFloat( Sigml::tObjectProperties::fEditablePropLODMediumDistanceName( ), 0.f, 0.f, 10000.f, 1.f, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCDCFloat( Sigml::tObjectProperties::fEditablePropLODFarDistanceName( ), 0.f, 0.f, 10000.f, 1.f, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObjectProperties::fEditablePropShowLODFromCameraName( ), false ) ) );
	}

	void tFxmlReferenceEntity::fCommonCtor( const tResourcePtr& fxResource )
	{
		if( mFxSystem )
		{
			mFxSystem->fDeleteImmediate( );
			mFxSystem.fRelease( );
		}

		// initialize fxb renderables
		mToolTip = Fxml::fFxbPathToFxml( fxResource->fGetResourceId( ).fGetPath( ) ).fCStr( );

		// until the object is loaded, set the dummy object's bounds as our own
		const Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		mDummyBox->fSetRgbaTint( Math::tVec4f( 0.1f, 1.0f, 0.2f, 0.5f ) );
		mDummyBox->fSetInvisible( false );

		mOnResourceLoaded.fFromMethod< tFxmlReferenceEntity, &tFxmlReferenceEntity::fOnResourceLoaded >( this );
		mFxSystem.fReset( new FX::tFxFileRefEntity( fxResource, -1, false, false, 0.0f, NULL, *Gfx::tVisibilitySetRefManager::fInstance( ).fBlankRef( ) ) );
		fxResource->fCallWhenLoaded( mOnResourceLoaded );
	}

	void tFxmlReferenceEntity::fOnResourceLoaded( tResource& theResource, b32 success )
	{
		const FX::tFxFile* fxFile = mFxSystem->fFxResource( )->fCast<FX::tFxFile>( );
		if( success && fxFile )
		{
			mFxSystem->fSpawnImmediate( *this );
			mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.125f ) );
			fUpdateStateTint( );
			mDummyBox->fSetRgbaTint( Math::tVec4f( mDummyBox->fRgbaTint( ).fXYZ( ), 0.125f ) );
		}
		else
		{
			mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.75f ) );
		}
	}

	tFxmlReferenceEntity::~tFxmlReferenceEntity( )
	{
	}

	tFilePathPtr tFxmlReferenceEntity::fResourcePath( ) const
	{
		return mFxSystem && mFxSystem->fFxResource( ) ? mFxSystem->fFxResource( )->fGetPath( ) : tFilePathPtr( );
	}

	void tFxmlReferenceEntity::fResetReference( const tFilePathPtr& newRefPath )
	{
		tResourceId rid;
		tResourcePtr sgResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( tResourceId::fMake<FX::tFxFile>( newRefPath ) );

		fCommonCtor( sgResource );
	}

	void tFxmlReferenceEntity::fRefreshDependents( const tResourcePtr& reloadedResource )
	{
		if( mFxSystem && mFxSystem->fFxResource( ) )
			fCommonCtor( mFxSystem->fFxResource( ) );
	}

	std::string tFxmlReferenceEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return mToolTip + " - " + name;
		return mToolTip;
	}

	std::string tFxmlReferenceEntity::fGetAssetPath( ) const
	{
		return fResourcePath().fCStr();
	}

	Sigml::tObjectPtr tFxmlReferenceEntity::fSerialize( b32 clone ) const
	{
		Sigml::tFxmlReferenceObject* o = new Sigml::tFxmlReferenceObject( );
		fSerializeBaseObject( o, clone );
		if( mFxSystem && mFxSystem->fFxResource( ) )
			o->mReferencePath = Fxml::fFxbPathToFxml( mFxSystem->fFxResource( )->fGetResourceId( ).fGetPath( ) );
		return Sigml::tObjectPtr( o );
	}

}

