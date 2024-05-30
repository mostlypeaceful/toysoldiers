#include "ToolsPch.hpp"
#include "tEditableObject.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"

// intersections
#include "Math/tIntersectionSphereFrustum.hpp"

// graphics
#include "Gfx/tViewport.hpp"
#include "Gfx/tDisplayList.hpp"

namespace Sig
{
	tEditableObject::tDummyObjectEntity::tDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox, b32 useSphereCollision )
		: Gfx::tRenderableEntity( batchPtr, objectSpaceBox )
		, mUseSphereCollision( useSphereCollision )
	{
	}
	void tEditableObject::tDummyObjectEntity::fApplyCreationFlags( const tEntityCreationFlags& creationFlags )
	{
		// do nothing, just in case
	}
	void tEditableObject::tDummyObjectEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		tEditableObject* eo = fFirstAncestorOfType< tEditableObject >( );
		if( !eo ) return;
		const Math::tRayf rayInLocal = ray.fTransform( fWorldToObject( ) );
		if( mUseSphereCollision )
			eo->fGetContainer( ).fGetDummySphereTemplate( ).fIntersectsRay( rayInLocal, hit.mT );
		else
			eo->fGetContainer( ).fGetDummyBoxTemplate( ).fIntersectsRay( rayInLocal, hit.mT );
	}
	b32	tEditableObject::tDummyObjectEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		tEditableObject* eo = fFirstAncestorOfType< tEditableObject >( );
		if( !eo ) return false;

		if( mUseSphereCollision )
		{
			// currently the sphere/frustum test is rough, and basically returns result like an AABB... oh well, may this will change
			const Math::tSpheref s = eo->fGetContainer( ).fGetDummySphereTemplate( ).fGetSphere( fObjectToWorld( ) );
			return Math::tIntersectionSphereFrustum<f32>( s, v ).fIntersects( );
		}
		else
			return tRenderableEntity::fIntersects( v );
	}



	tEditableObject::tEditableObject( tEditableObjectContainer& container )
		: mContainer( container )
		, mInContainer( false )
		, mSelected( false )
		, mGuid( container.fNextGuid( ) )
		, mState( cStateShown )
		, mLayer( "" )
	{
		// add editable properties
		mOnPropertyChanged.fFromMethod< tEditableObject, &tEditableObject::fNotifyPropertyChanged >( this );
		mEditableProperties.mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyString( Sigml::tObject::fEditablePropObjectName( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( Sigml::tObject::fEditablePropScriptName( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropGroundRelative( ), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( Sigml::tObject::fEditablePropGroundOffset( ), 0.f, -9999.f, +9999.f, 0.01f, 2 ) ) );

		// compute local space box for dummy object and selection box
		const Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		const Math::tAabbf localSpaceSphereBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );

		// create dummy object entity
		mDummyBox.fReset( new tDummyObjectEntity( container.fGetDummyBoxTemplate( ).fGetRenderBatch( ), localSpaceBox ) );
		mDummyBox->fSpawnImmediate( *this );
		mDummyBox->fSetInvisible( true );
		mDummySphere.fReset( new tDummyObjectEntity( container.fGetDummySphereTemplate( ).fGetRenderBatch( ), localSpaceSphereBox, true ) );
		mDummySphere->fSpawnImmediate( *this );
		mDummySphere->fSetInvisible( true );

		// create selection box entity
		mSelectionBox.fReset( new Gfx::tRenderableEntity( container.fGetSelectedBoxTemplate( ).fGetRenderBatch( ), localSpaceBox ) );
		mSelectionBox->fSetRgbaTint( Math::tVec4f( 0.5f, 0.5f, 0.5f, 1.f ) );
		mSelectionBox->fSpawnImmediate( *this );
		mSelectionBox->fSetInvisible( true );
	}

	tEditableObject::~tEditableObject( )
	{
		fRemoveFromWorld( );
	}

	void tEditableObject::fAddToWorld( )
	{
		if( !mInContainer )
		{
			mInContainer = true;
			mContainer.fInsert( fToSmartPtr( ) );
		}
	}

	void tEditableObject::fRemoveFromWorld( )
	{
		if( mInContainer )
		{
			mInContainer = false;
			fDisableSelectionBox( );
			fSetSelected( false );
			mContainer.fRemove( this );
		}
	}


	void tEditableObject::fEnableSelectionBox( )
	{
		mSelectionBox->fSetInvisible( false );
	}

	void tEditableObject::fDisableSelectionBox( )
	{
		mSelectionBox->fSetInvisible( true );
	}

	void tEditableObject::fSetState( tState newState )
	{
		if( newState == mState )
			return;
		fPreStateChange( );
		fRemoveFromWorld( );
		mState = newState;
		fAddToWorld( );
		fPostStateChange( );
		fUpdateStateTint( );
	}

	void tEditableObject::fHide( b32 hide )
	{
		fSetState( hide ? cStateHidden : cStateShown );
	}

	void tEditableObject::fFreeze( b32 freeze )
	{
		fSetState( freeze ? cStateFrozen : cStateShown );
	}

	void tEditableObject::fSetSelected( b32 sel )
	{
		if( mSelected && !sel )
		{
			fDisableSelectionBox( );
			mSelectionBox->fSetRgbaTint( Math::tVec4f( 0.5f, 0.5f, 0.5f, 1.f ) );
		}
		else if( !mSelected && sel && fIsShown( ) )
		{
			fEnableSelectionBox( );
			mSelectionBox->fSetRgbaTint( Math::tVec4f( 1.0f ) );
		}
		mSelected = sel && fIsShown( );
	}

	std::string tEditableObject::fGetName( ) const
	{
		std::string name="";
		tEditablePropertyPtr* find = mEditableProperties.fFind( Sigml::tObject::fEditablePropObjectName( ) );
		if( find ) (*find)->fGetData( name );
		return name;
	}

	std::string tEditableObject::fGetScriptName( ) const
	{
		std::string script="";
		tEditablePropertyPtr* find = mEditableProperties.fFind( Sigml::tObject::fEditablePropScriptName( ) );
		if( find ) (*find)->fGetData( script );
		return script;
	}

	std::string tEditableObject::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		std::string name = fGetName( );
		return name;
	}

	void tEditableObject::fComputeDisplayStats( Gfx::tDisplayStats& displayStatsOut ) const
	{
		fComputeDisplayStats( *this, displayStatsOut );
	}

	void tEditableObject::fComputeDisplayStats( const tEntity& entity, Gfx::tDisplayStats& displayStatsOut ) const
	{
		Gfx::tRenderableEntity* r = entity.fDynamicCast< Gfx::tRenderableEntity >( );
		if( r && r->fRenderBatch( ) && !r->fIsHelper( ) )
		{
			if( !r->fDynamicCast< tDummyObjectEntity >( ) )
			{
				Gfx::tDisplayStats d;
				r->fComputeDisplayStats( d );
				displayStatsOut.fCombine( d );
			}
		}

		for( u32 i = 0; i < entity.fChildCount( ); ++i )
			fComputeDisplayStats( *entity.fChild( i ), displayStatsOut );
	}

	tEntityPtr tEditableObject::fClone( )
	{
		Sigml::tObjectPtr sigmlObject = fSerialize( true );

		if( sigmlObject.fNull( ) )
			return tEntityPtr( );

		tEditableObject* eo = sigmlObject->fCreateEditableObject( mContainer );
		sigassert( eo );

		//eo->mGuid = mContainer.fNextGuid( );

		eo->fMoveTo( fObjectToWorld( ) );
		eo->fAddToWorld( );

		return tEntityPtr( eo );
	}

	Math::tVec4f tEditableObject::fCurrentStateTint( ) const
	{
		const Math::tVec4f normalTint = Math::tVec4f( 1.f );
		const Math::tVec4f frozenTint = mContainer.fLayerColors( )[ mLayer ];
		return fIsFrozen( ) ? frozenTint : normalTint;
	}

	void tEditableObject::fUpdateStateTint( )
	{
		const Math::tVec4f curStateTint = fCurrentStateTint( );
		for( u32 i = 0; i < fChildCount( ); ++i )
			if( fChild( i ) != mSelectionBox.fGetRawPtr( ) )
				fUpdateStateTint( *fChild( i ), curStateTint );
	}

	void tEditableObject::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		Gfx::tRenderableEntity* r = entity.fDynamicCast< Gfx::tRenderableEntity >( );
		if( r )
		{
			if( r == mDummyBox || r == mDummySphere )
				r->fSetRgbaTint( Math::tVec4f( rgbaTint.fXYZ( ), 0.5f ) );
			else
				r->fSetRgbaTint( rgbaTint );
		}

		for( u32 i = 0; i < entity.fChildCount( ); ++i )
			fUpdateStateTint( *entity.fChild( i ), rgbaTint );
	}

	void tEditableObject::fSerializeBaseObject( Sigml::tObject* baseObject, b32 clone ) const
	{
		baseObject->mGuid = clone ? mContainer.fNextGuid( ) : mGuid;
		baseObject->mClone = clone;
		baseObject->mHidden = fIsHidden( );
		baseObject->mFrozen = fIsFrozen( );
		baseObject->mLayer = mLayer;
		baseObject->mXform = fObjectToWorld( );
		baseObject->mEditableProperties.fUnion( mEditableProperties );
	}

	void tEditableObject::fDeserializeBaseObject( const Sigml::tObject* baseObject )
	{
		mGuid = baseObject->mClone ? baseObject->mGuid : mContainer.fDeserializeGuid( baseObject->mGuid );

		if( baseObject->mHidden )
			mState = cStateHidden;
		else if( baseObject->mFrozen )
			mState = cStateFrozen;
		else
			mState = cStateShown;
		mLayer = baseObject->mLayer;
		fMoveTo( baseObject->mXform );
		mEditableProperties.fUnion( baseObject->mEditableProperties );
	}

	Math::tAabbf tEditableObject::fObjectSpaceBox( ) const
	{
		return mSelectionBox->fObjectSpaceBox( );
	}

	Math::tAabbf tEditableObject::fWorldSpaceBox( ) const
	{
		return mSelectionBox->fObjectSpaceBox( ).fTransform( fObjectToWorld( ) );
	}

	Math::tObbf tEditableObject::fWorldSpaceObb( ) const
	{
		return Math::tObbf( fObjectSpaceBox( ), fObjectToWorld( ) );
	}

	void tEditableObject::fSnapToGround( const tDynamicArray< tEntity* >& toIgnore )
	{
		const Math::tVec3f p = fObjectToWorld( ).fGetTranslation( );

		const f32 offset = mEditableProperties.fGetValue<f32>( Sigml::tObject::fEditablePropGroundOffset( ), 0.f );
		
		f32 tDown = 1.1f;
		const Math::tRayf r( p + ( 1.0f - offset ) * Math::tVec3f::cYAxis, -Math::cInfinity * Math::tVec3f::cYAxis );
		if( mContainer.fPick( r, &tDown, toIgnore.fBegin( ), toIgnore.fCount( ) ) )
			fMoveTo( r.fEvaluate( tDown ) + offset * Math::tVec3f::cYAxis );
		else
		{
			// didn't hit anything, try again this time from a higher vantage point
			const Math::tRayf r( p + ( 100.0f - offset ) * Math::tVec3f::cYAxis, -Math::cInfinity * Math::tVec3f::cYAxis );
			if( mContainer.fPick( r, &tDown, toIgnore.fBegin( ), toIgnore.fCount( ) ) )
				fMoveTo( r.fEvaluate( tDown ) + offset * Math::tVec3f::cYAxis );
		}
	}

	void tEditableObject::fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max )
	{
		const Math::tAabbf objSpaceBox = Math::tAabbf( min, max );
		mSelectionBox->fSetObjectSpaceBox( objSpaceBox );
		mSelectionBox->fSetParentRelativeXform( objSpaceBox.fAdjustMatrix( Math::tMat3f::cIdentity, 0.25f ) );
	}

	void tEditableObject::fNotifyPropertyChanged( tEditableProperty& property )
	{
		mContainer.fGetActionStack( ).fForceSetDirty( true );

		if( property.fGetName( ) == Sigml::tObject::fEditablePropCastShadow( ) )
		{
			fUpdateShadowCasting( );
		}
	}

	struct tUpdateShadowCasting
	{
		b32 mCast;
		tGrowableArray<tEntity*> mIgnore;
		tUpdateShadowCasting( b32 cast ) : mCast( cast ) { }
		b32 operator()( tEntity& entity ) const
		{
			for( u32 i = 0; i < mIgnore.fCount( ); ++i )
			{
				if( &entity == mIgnore[ i ] )
					return false;
			}

			Gfx::tRenderableEntity* renderable = entity.fDynamicCast< Gfx::tRenderableEntity >( );
			if( renderable )
				renderable->fSetCastsShadow( mCast );
			return false;
		}
	};


	void tEditableObject::fUpdateShadowCasting( )
	{
		const b32 b = mEditableProperties.fGetValue( Sigml::tObject::fEditablePropCastShadow( ), false );
		tUpdateShadowCasting functor( b );
		functor.mIgnore.fPushBack( mDummyBox.fGetRawPtr( ) );
		functor.mIgnore.fPushBack( mDummySphere.fGetRawPtr( ) );
		functor.mIgnore.fPushBack( mSelectionBox.fGetRawPtr( ) );
		fForEachDescendent( functor );
	}

}

