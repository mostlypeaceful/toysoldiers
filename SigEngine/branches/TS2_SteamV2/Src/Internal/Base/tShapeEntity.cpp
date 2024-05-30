#include "BasePch.hpp"
#include "tSceneGraph.hpp"
#include "tShapeEntity.hpp"

#include "Math/tIntersectionRayObb.hpp"
#include "Math/tIntersectionRaySphere.hpp"
#include "Math/tIntersectionAabbObb.hpp"
#include "Math/tIntersectionAabbSphere.hpp"
#include "Math/tIntersectionSphereObb.hpp"

namespace Sig
{

	tShapeEntityDef::tShapeEntityDef( )
		: mShapeType( cShapeTypeBox )
		, mStateMask( 0xFFFF )
		, pad2( 0 )
		, pad3( 0.f )
		, pad4( 0.f )
	{
	}

	tShapeEntityDef::tShapeEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tShapeEntityDef::~tShapeEntityDef( )
	{
	}

	void tShapeEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		tShapeEntity* entity = NEW tShapeEntity( this, mBounds, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *entity, parent, creationFlags );
	}

	const u32 tShapeEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	tShapeEntity::tShapeEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: tSpatialEntity( objectSpaceBox, entityDef->mStateMask )
		, mEntityDef( entityDef )
		, mShapeType( entityDef->mShapeType )
		, mEnabled( fTestBits( entityDef->mStateMask, (1<<0) ) )
	{
		fMoveTo( objectToWorld );
	}

	tShapeEntity::tShapeEntity( const Math::tAabbf& objectSpaceBox, tShapeEntityDef::tShapeType shape )
		: tSpatialEntity( Math::tAabbf( -1.f, +1.f ) )
		, mEntityDef( NULL )
		, mShapeType( shape )
		, mStateIndex( -1 )
		, mEnabled( true )
	{
		Math::tMat3f objectToWorld = Math::tMat3f::cIdentity;
		objectToWorld.fSetTranslation( objectSpaceBox.fComputeCenter( ) );
		objectToWorld.fScaleLocal( 0.5f * objectSpaceBox.fComputeDiagonal( ) );

		fMoveTo( objectToWorld );
	}

	void tShapeEntity::fPropagateSkeleton( tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	b32 tShapeEntity::fContains( const Math::tVec3f& point ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			return mBox.fContains( point );
		case tShapeEntityDef::cShapeTypeSphere:
			return mSphere.fContains( point );
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return false;
	}

	b32 tShapeEntity::fIntersects( const tShapeEntity& otherShape ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			return otherShape.fIntersects( mBox );
		case tShapeEntityDef::cShapeTypeSphere:
			return otherShape.fIntersects( mSphere );
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return false;
	}

	namespace
	{
		Math::tVec3f fNormalFromPointOnBox( const Math::tVec3f& point, const Math::tObbf& obb )
		{
			for( u32 i = 0; i < 3; ++i )
			{
				const Math::tPlanef p0 = Math::tPlanef( obb.mAxes[i], obb.mCenter + obb.mAxes[i] * obb.mExtents[i] );
				if( fAbs( p0.fSignedDistance( point ) ) < 0.1f )
					return p0.fGetNormalUnit( );

				const Math::tPlanef p1 = Math::tPlanef( -obb.mAxes[i], obb.mCenter - obb.mAxes[i] * obb.mExtents[i] );
				if( fAbs( p1.fSignedDistance( point ) ) < 0.1f )
					return p1.fGetNormalUnit( );
			}

			return Math::tVec3f::cYAxis;
		}
	}

	void tShapeEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( !mEnabled )
			return;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			{
				Math::tIntersectionRayObb<f32> i( ray, mBox );
				if( i.fIntersects( ) )
				{
					hit.mT = i.fT( );

					const Math::tVec3f p = ray.fEvaluate( hit.mT );
					hit.mN = fNormalFromPointOnBox( p, mBox );
				}
			}
			break;
		case tShapeEntityDef::cShapeTypeSphere:
			{
				Math::tIntersectionRaySphere<f32> i( ray, mSphere );
				if( i.fIntersects( ) )
				{
					hit.mT = i.fT( );
					hit.mN = ( ray.fEvaluate( hit.mT ) - mSphere.fCenter( ) ).fNormalizeSafe( Math::tVec3f::cYAxis );
				}
			}
			break;
		default:
			sigassert( !"invalid shape type" );
			break;
		}
	}

	b32 tShapeEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			break;
		case tShapeEntityDef::cShapeTypeSphere:
			break;
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return tSpatialEntity::fIntersects( v );
	}

	b32 tShapeEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			return Math::tIntersectionAabbObb<f32>( mBox, v ).fIntersects( );
		case tShapeEntityDef::cShapeTypeSphere:
			return Math::tIntersectionAabbSphere<f32>( mSphere, v ).fIntersects( );
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return tSpatialEntity::fIntersects( v );
	}

	b32 tShapeEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			break;
		case tShapeEntityDef::cShapeTypeSphere:
			return Math::tIntersectionSphereObb<f32>( mSphere, v ).fIntersects( );
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return tSpatialEntity::fIntersects( v );
	}

	b32 tShapeEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( !mEnabled )
			return false;

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			return Math::tIntersectionSphereObb<f32>( mBox, v ).fIntersects( );
		case tShapeEntityDef::cShapeTypeSphere:
			return mSphere.fIntersects( v );
		default:
			sigassert( !"invalid shape type" );
			break;
		}
		return tSpatialEntity::fIntersects( v );
	}

	void tShapeEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tSpatialEntity::fOnMoved( recomputeParentRelative );

		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox: mBox = fComputeBox( ); break;
		case tShapeEntityDef::cShapeTypeSphere: mSphere = fComputeSphere( ); break;
		default: sigassert( !"invalid shape type" ); break;
		}
	}

	Math::tObbf tShapeEntity::fParentRelativeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fParentRelative( ) );
	}

	Math::tSpheref tShapeEntity::fParentRelativeSphere( ) const
	{
		const Math::tMat3f& xform = fParentRelative( );
		return Math::tSpheref(
			xform.fGetTranslation( ),
			fMax( xform.fXAxis( ).fLength( ), xform.fYAxis( ).fLength( ), xform.fZAxis( ).fLength( ) ) );
	}

	Math::tVec3f tShapeEntity::fClosestPoint( const Math::tVec3f& point ) const
	{
		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			return mBox.fClosestPoint( point );
		case tShapeEntityDef::cShapeTypeSphere:
			{
				Math::tVec3f delta = point - mSphere.fCenter( );
				f32 distance;
				delta.fNormalizeSafe( Math::tVec3f::cZeroVector, distance );

				return mSphere.fCenter( ) + delta * fMin( distance, mSphere.fRadius( ) );
			}
		default:
			sigassert( !"invalid shape type" );
			return Math::tVec3f::cZeroVector;
			break;
		}	
	}

	Math::tVec3f tShapeEntity::fRandomPoint( tRandom& rand ) const
	{
		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			{
				const f32 x = rand.fFloatMinusOneToOne( );
				const f32 y = rand.fFloatMinusOneToOne( );
				const f32 z = rand.fFloatMinusOneToOne( );
				return fObjectToWorld( ).fXformPoint( Math::tVec3f( x, y, z ) );
			} break;			
		case tShapeEntityDef::cShapeTypeSphere:
			{
				const Math::tVec3f randPoint = rand.fVecNorm<Math::tVec3f>( ) * rand.fFloatZeroToOne( );
				return fObjectToWorld( ).fXformPoint( randPoint );
			} break;
		default:
			sigassert( !"tShapeEntity::fRandomPoint - invalid shape type" );
		}
		return fObjectToWorld( ).fGetTranslation( );
	}

#ifdef sig_devmenu
	void tShapeEntity::fDebugRender( ) const
	{
		const Math::tVec4f color( 1, 1, 1, 0.5f );
		switch( mShapeType )
		{
		case tShapeEntityDef::cShapeTypeBox:
			{
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fComputeBox( ), color );
			} break;
		case tShapeEntityDef::cShapeTypeSphere:
			{
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fComputeSphere( ), color );
			} break;
		default:
			log_warning( 0,  "tShapeEntity::fDebugRender - invalid shape type" );
		}
		fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fObjectToWorld( ), 1.f );
	}
#endif//sig_devmenu

	Math::tObbf tShapeEntity::fComputeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fObjectToWorld( ) );
	}

	Math::tSpheref tShapeEntity::fComputeSphere( ) const
	{
		const Math::tMat3f& xform = fObjectToWorld( );
		return Math::tSpheref(
			xform.fGetTranslation( ),
			fMax( xform.fXAxis( ).fLength( ), xform.fYAxis( ).fLength( ), xform.fZAxis( ).fLength( ) ) );
	}

}

namespace Sig
{
	void tShapeEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tShapeEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("ShapeType"), &tShapeEntity::fShapeTypeAsInt)
			.Func(_SC("GetBox"), &tShapeEntity::fComputeBox)
			.Func(_SC("GetSphere"), &tShapeEntity::fComputeSphere)
			;
		vm.fRootTable( ).Bind(_SC("ShapeEntity"), classDesc);

		vm.fConstTable( ).Const( "SHAPE_ENTITY_BOX", ( int )tShapeEntityDef::cShapeTypeBox );
		vm.fConstTable( ).Const( "SHAPE_ENTITY_SPHERE", ( int )tShapeEntityDef::cShapeTypeSphere );
	}
}
