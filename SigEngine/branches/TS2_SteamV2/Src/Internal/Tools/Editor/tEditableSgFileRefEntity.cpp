#include "ToolsPch.hpp"
#include "tEditableSgFileRefEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSceneGraphFile.hpp"
#include "tMeshEntity.hpp"
#include "tAssetPluginDll.hpp"
#include "iSigEdPlugin.hpp"

namespace Sig
{
	namespace
	{
		static const char* fEditablePropStateToDisplay( ) { return "Display.StateIndex"; }
	}

	tEditableSgFileRefEntity::tEditableSgFileRefEntity( tEditableObjectContainer& container, const tResourcePtr& sgResource )
		: tEditableObject( container )
		, mLastDisplayIndex( -1 )
	{
		fAddEditableProperties( );

		fCommonCtor( sgResource );
	}

	tEditableSgFileRefEntity::tEditableSgFileRefEntity( tEditableObjectContainer& container, const Sigml::tSigmlReferenceObject& sigmlObject )
		: tEditableObject( container )
		, mLastDisplayIndex( -1 )
	{
		fAddEditableProperties( );

		fDeserializeBaseObject( &sigmlObject );
		mEditableProperties.fRemove( "Render.AutoFadeOut" ); // deprecated

		tResourceId rid;
		tResourcePtr sgResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( sigmlObject.fConstructResourceId( rid ) );

		fCommonCtor( sgResource );
	}

	void tEditableSgFileRefEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropInvisible( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropCastShadow( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropReceiveShadow( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropDisableContextAnims( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropStateToDisplay( ), 0, -1, 254, 1, 0 ) ) );
		Sigml::tObjectProperties::fAddFadeSettingsEditableProperties( mEditableProperties );
	}

	void tEditableSgFileRefEntity::fCommonCtor( const tResourcePtr& sgResource )
	{
		if( mSgFileRefEntity )
		{
			mSgFileRefEntity->fDeleteImmediate( );
			mSgFileRefEntity.fRelease( );
		}

		mSgResource = sgResource;

		// initialize sigb renderables
		mLastDisplayIndex = -1;
		mToolTip = Sigml::fSigbPathToSigml( sgResource->fGetPath( ) ).fCStr( );

		// until the object is loaded, set the dummy object's bounds as our own
		const Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		mDummyBox->fSetRgbaTint( Math::tVec4f( 0.1f, 1.0f, 0.2f, 0.5f ) );
		mDummyBox->fSetInvisible( false );

		mOnResourceLoaded.fFromMethod< tEditableSgFileRefEntity, &tEditableSgFileRefEntity::fOnResourceLoaded >( this );
		mSgFileRefEntity.fReset( new tSgFileRefEntity( sgResource ) );
		sgResource->fCallWhenLoaded( mOnResourceLoaded );
	}

	namespace
	{
		static u32 fRenderableEntityCount( tEntity& root )
		{
			u32 o = 0;
			if( root.fDynamicCast< Gfx::tRenderableEntity >( ) )
				++o;
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				o += fRenderableEntityCount( *root.fChild( i ) );
			return o;
		}
	}

	void tEditableSgFileRefEntity::fOnResourceLoaded( tResource& theResource, b32 success )
	{
		if( !mSgFileRefEntity )
		{
			return;
		}

		const tSceneGraphFile* sgFile = mSgFileRefEntity->fSgResource( )->fCast< tSceneGraphFile >( );
		if( success && sgFile )
		{
			mSgFileRefEntity->fSpawnImmediate( *this );

			if( fRenderableEntityCount( *mSgFileRefEntity ) > 0 )
			{
				fSetLocalSpaceMinMax( sgFile->mBounds.mMin, sgFile->mBounds.mMax );
				mDummyBox->fSetInvisible( true );
			}
			else
			{
				const Math::tAabbf objSpaceBox = mSgFileRefEntity->fCombinedObjectSpaceBox( );
				fSetLocalSpaceMinMax( objSpaceBox.mMin, objSpaceBox.mMax );
				mDummyBox->fSetParentRelativeXform( objSpaceBox.fAdjustMatrix( Math::tMat3f::cIdentity, 0.0f ) );
			}

			fUpdateStateTint( );

			tGrowableArray<f32> radii;
			tSigEdPluginInputOutput output( mSgFileRefEntity, radii );
			tAssetPluginDllDepot::fInstance( ).fForEachPlugin( output );

			for( u32 i = 0; i < radii.fCount( ); ++i )
			{
				Gfx::tRenderableEntityPtr newCage;
				newCage.fReset( new Gfx::tRenderableEntity( 
					mContainer.fGetSphereCageSparseTemplate( ).fGetRenderBatch( ), 
					mContainer.fGetDummyBoxTemplate( ).fGetBounds( ) ) );
				Math::tMat3f mLil( Math::tMat3f::cIdentity );
				mLil.fScaleLocal( radii[i] );

				newCage->fSpawnImmediate( *this );
				newCage->fSetParentRelativeXform( mLil );

				mGameDebugGeom.fPushBack( newCage );
			}
		}
		else
		{
			mSgFileRefEntity.fRelease( );
			mDummyBox->fSetInvisible( false );
			mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.75f ) );
		}

		const s32 displayIndex = fGetDisplayIndex( );
		fUpdateDisplayedStates( displayIndex );
		mLastDisplayIndex = displayIndex;

		fUpdateShadowCasting( );
	}

	tEditableSgFileRefEntity::~tEditableSgFileRefEntity( )
	{
	}

	tFilePathPtr tEditableSgFileRefEntity::fResourcePath( ) const
	{
		if( mSgResource.fNull( ) )
			return tFilePathPtr( );
		return mSgResource->fGetPath( );
	}

	//------------------------------------------------------------------------------
	const tResourcePtr & tEditableSgFileRefEntity::fResource( ) const
	{
		return mSgResource;
	}

	b32 tEditableSgFileRefEntity::fIsEditable( ) const
	{
		const tFilePathPtr resPath = fResourcePath( );
		return StringUtil::fCheckExtension( resPath.fCStr( ), ".sigb" );
	}

	b32 tEditableSgFileRefEntity::fIsRedBox( ) const
	{
		return !mSgFileRefEntity;
	}

	void tEditableSgFileRefEntity::fResetReference( const tFilePathPtr& newRefPath )
	{
		tResourceId rid;
		tResourcePtr sgResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( tResourceId::fMake<tSceneGraphFile>( newRefPath ) );

		fCommonCtor( sgResource );
	}

	void tEditableSgFileRefEntity::fRefreshDependents( const tResourcePtr& reloadedResource, b32 unloadOnly )
	{
		tResourcePtr sgResource = mSgResource;

		if( unloadOnly )
		{
			if( mSgFileRefEntity )
			{
				tResource::tVisitedList visitedList0,visitedList1;
				if( sgResource == reloadedResource || 
					sgResource->fIsSubResource( *reloadedResource, visitedList0 ) || 
					reloadedResource->fIsSubResource( *sgResource, visitedList1 ) )
				{
					mSgFileRefEntity->fDeleteImmediate( );
					mSgFileRefEntity.fRelease( );
				}
			}
		}
		else if( !mSgFileRefEntity )
			fCommonCtor( sgResource );
	}

	std::string tEditableSgFileRefEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) )
			return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return mToolTip + " - " + name;
		return mToolTip;
	}

	Sigml::tObjectPtr tEditableSgFileRefEntity::fSerialize( b32 clone ) const
	{
		Sigml::tSigmlReferenceObject* o = new Sigml::tSigmlReferenceObject( );
		fSerializeBaseObject( o, clone );
		if( mSgResource )
			o->mReferencePath = Sigml::fSigbPathToSigml( mSgResource->fGetPath( ) );
		return Sigml::tObjectPtr( o );
	}

	void tEditableSgFileRefEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName( ) == fEditablePropStateToDisplay( ) )
		{
			const u32 displayIndex = fGetDisplayIndex( );
			fUpdateDisplayedStates( displayIndex );
			mLastDisplayIndex = displayIndex;
		}
	}

	s32 tEditableSgFileRefEntity::fGetDisplayIndex( ) const
	{
		return mEditableProperties.fGetValue<s32>( fEditablePropStateToDisplay( ), 0 );
	}

	void tEditableSgFileRefEntity::fUpdateDisplayedStates( u32 displayIndex )
	{
		tMeshEntity::tChangeMeshState change( mLastDisplayIndex, displayIndex );
		change.fChangeState( *this );
	}

}

