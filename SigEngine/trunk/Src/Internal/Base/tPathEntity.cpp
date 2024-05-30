#include "BasePch.hpp"
#include "tPathEntity.hpp"

namespace Sig
{
	tPathEntityDef::tPathEntityDef( )
		: mGuid( 0 )
	{
	}

	tPathEntityDef::tPathEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mNextPoints( cNoOpTag )
	{
	}

	tPathEntityDef::~tPathEntityDef( )
	{
	}

	void tPathEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		tPathEntity* entity = NEW_TYPED( tPathEntity )( this, mBounds, mObjectToLocal );

		fApplyProperties( *entity, params.mCreationFlags );
		fEntityOnCreate( *entity );
		entity->fSpawn( params.mParent );
		fEntityOnChildrenCreate( *entity );
	}

	tPathEntity::tPathEntity( const tPathEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: mEntityDef( entityDef )
	{
		fMoveTo( objectToWorld );
	}

	void tPathEntity::fAfterSiblingsHaveBeenCreated( )
	{
		for( u32 i = 0; i < fParent( )->fChildCount( ); ++i )
		{
			tPathEntity* sibling = fParent( )->fChild( i )->fDynamicCast< tPathEntity >( );
			if( sibling && sibling != this )
			{
				if( mEntityDef->mNextPoints.fFind( sibling->fEntityDef( )->mGuid ) )
				{
					mNextPoints.fPushBack( sibling );
					sibling->mPrevPoints.fPushBack( this );
				}
			}
		}

		mEntityDef->fEntityOnSiblingsCreate( *this );
	}

	void tPathEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	b32 tPathEntity::fTraversePath( f32 distance, Math::tVec3f& out ) const
	{
		if( fNextPointCount( ) == 0 )
		{
			out = fObjectToWorld( ).fGetTranslation( );
			return false;
		}
		else
		{
			Math::tVec3f myPos = fObjectToWorld( ).fGetTranslation( );
			Math::tVec3f delta = fNextPoint( 0 )->fObjectToWorld( ).fGetTranslation( ) - myPos;
			f32 len;
			delta.fNormalizeSafe( Math::tVec3f::cZeroVector, len );

			if( len >= distance )
			{
				out = myPos + delta * distance;
				return true;
			}
			else
				return fNextPoint( 0 )->fTraversePath( distance - len, out );
		}

		//shouldn't get here
		return false;
	}

	namespace 
	{
		static void fForEachNextPt( tPathEntity* entity, Sqrat::Function func )
		{
			const tPathEntity::tConnectionsList& nextPoints = entity->fNextPathPoints( );
			for( u32 i = 0; i < nextPoints.fCount( ); ++i )
				func.Execute( nextPoints[ i ] );
		}

		static tPathEntity* fAsPathEntity( const tEntity* obj )
		{
			return obj->fDynamicCast< tPathEntity >( );
		}
	}

	void tPathEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tPathEntity, tEntity, Sqrat::NoCopy<tPathEntity> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("PrevPointCount"), &tPathEntity::fPrevPointCount)
			.Func(_SC("PrevPoint"), &tPathEntity::fPrevPoint)
			.Func(_SC("NextPointCount"), &tPathEntity::fNextPointCount)
			.Func(_SC("NextPoint"), &tPathEntity::fNextPoint)
			.StaticFunc(_SC("AsPathEntity"), &fAsPathEntity)
			.GlobalFunc(_SC("ForEachNextPt"), &fForEachNextPt)
			;

		vm.fRootTable( ).Bind(_SC("PathEntity"), classDesc);
	}
}

