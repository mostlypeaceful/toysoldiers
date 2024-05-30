#include "BasePch.hpp"
#include "tProximity.hpp"
#include "tSceneGraph.hpp"
#include "tSpatialEntity.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( bool, Debug_Proximity_Render, false );

	namespace
	{
		struct tFilter
		{
			inline static b32 fFilter( const tProximity::tPropertyGroup& filter, tEntity* test )
			{
				if( test->fHasGameTagsAny( filter.fTags( ) ) )
					return true;
				for( u32 i = 0; i < filter.fPropsAny( ).fCount( ); ++i )
					if( test->fQueryEnumValue( filter.fPropsAny( )[ i ].mEnumKey ) == filter.fPropsAny( )[ i ].mEnumValue )
						return true;
				return false;
			}
			inline static b32 fLogicFilter( const tProximity::tPropertyGroup& filter, tEntity* test )
			{
				for( u32 i = 0; i < filter.fPropsMustHave( ).fCount( ); ++i )
				{
					u32 enumVal = test->fQueryEnumValue( filter.fPropsMustHave( )[ i ].mEnumKey );
					if( enumVal != filter.fPropsMustHave( )[ i ].mEnumValue )
						return false;
				}
				return fFilter( filter, test );
			}
		};
		struct tFilter_Inherited
		{
			inline static b32 fFilter( const tProximity::tPropertyGroup& filter, tEntity* test )
			{
				if( test->fHasGameTagsAnyInherited( filter.fTags( ) ) )
					return true;
				for( u32 i = 0; i < filter.fPropsAny( ).fCount( ); ++i )
				{
					u32 compare = filter.fPropsAny( )[ i ].mEnumValue;
					u32 value = test->fQueryEnumValueInherited( filter.fPropsAny( )[ i ].mEnumKey );
					if( value == compare || (value != ~0 && compare == ~0) )
						return true;
				}
				return false;
			}
			inline static b32 fLogicFilter( const tProximity::tPropertyGroup& filter, tEntity* test )
			{
				for( u32 i = 0; i < filter.fPropsMustHave( ).fCount( ); ++i )
				{
					u32 enumVal = test->fQueryEnumValueInherited( filter.fPropsMustHave( )[ i ].mEnumKey );
					if( enumVal != filter.fPropsMustHave( )[ i ].mEnumValue )
						return false;
				}
				return fFilter( filter, test );
			}
		};
		template<class tVolume, class tFilterFunction>
		struct tFindEntityShapeCallback : public tEntityBVH::tIntersectVolumeCallback<tVolume>, public tFilterFunction
		{
			const tEntity* mIgnore;
			tProximity& mProximity;
			b32 mFullTest;

			tFindEntityShapeCallback( const tEntity* ignore, tProximity& filteredList, b32 fullTest ) 
				: mIgnore( ignore ), mProximity( filteredList ), mFullTest( fullTest ) { }
			void operator()( const tVolume& v, tEntityBVH::tObjectPtr octreeObject, b32 aabbWhollyContained ) const
			{
				tSpatialEntity* test = static_cast< tSpatialEntity* >( octreeObject );

				if( mIgnore && test == mIgnore || test->fIsAncestorOfMine( *mIgnore ) )
					return;
				if( mProximity.fFilter( ).fSet( ) && !tFilterFunction::fFilter( mProximity.fFilter( ), test ) )
					return;
				if( !fQuickAabbTest( v, octreeObject, aabbWhollyContained ) )
					return;
				if( mFullTest && !test->fIntersects( v ) )
					return;

				mProximity.fAddEntity( test );
			}
		};

		template<class tVolume>
		void fIntersectVolumeWithSceneGraph( const tVolume& volume, const tSceneGraph& sg, const tEntity& parent, tProximity& proximity )
		{
			const b32 fullTest = true; // TODO parameterize somehow

			// only ignore entity if it's not the root entity of the scene graph
			// (used as a sort of dummy entity for proximity queries when there's no other suitable entity)
			const tEntity* ignore = ( &sg.fRootEntity( ) == &parent ) ? 0 : &parent;

			const tDynamicArray<u32>& spatialSetIndices = proximity.fSpatialSetIndices( );

			if( proximity.fQueryInheritedProperties( ) )
			{
				if( spatialSetIndices.fCount( ) == 0 )
					sg.fIntersect( volume, tFindEntityShapeCallback<tVolume,tFilter_Inherited>( ignore, proximity, fullTest ) );
				else
				{
					for( u32 i = 0; i < spatialSetIndices.fCount( ); ++i )
						sg.fIntersect( volume, tFindEntityShapeCallback<tVolume,tFilter_Inherited>( ignore, proximity, fullTest ), spatialSetIndices[ i ] );
				}
			}
			else
			{
				if( spatialSetIndices.fCount( ) == 0 )
					sg.fIntersect( volume, tFindEntityShapeCallback<tVolume,tFilter>( ignore, proximity, fullTest ) );
				else
				{
					for( u32 i = 0; i < spatialSetIndices.fCount( ); ++i )
						sg.fIntersect( volume, tFindEntityShapeCallback<tVolume,tFilter>( ignore, proximity, fullTest ), spatialSetIndices[ i ] );
				}
			}
		}
	}



	tProximity::tShape::tShape( tShapeType type, const tAabbf& localSpaceBox ) 
		: mLocalSpaceBox( localSpaceBox )
		, mRadius( 0.5f * localSpaceBox.fComputeDiagonal( ).fMaxMagnitude( ) )
		, mType( type )
	{
	}
	void tProximity::tShape::fIntersect( const tSceneGraph& sg, const tEntity& parent, const Math::tMat3f& xform, tProximity& proximity )
	{
		if( fEqual( mRadius, 0.f ) )
			return; // too small

		switch( mType )
		{
		case cShapeAabb:
			{
				const tAabbf volume = fToAabb( xform );
				sync_event_v_c( volume, tSync::cSCProximity );
				fIntersectVolumeWithSceneGraph<tAabbf>( volume, sg, parent, proximity );
			}
			break;
		case cShapeObb:
			{
				const tObbf volume = fToObb( xform );
				sync_event_v_c( volume.mCenter, tSync::cSCProximity );
				sync_event_v_c( volume.mExtents, tSync::cSCProximity );
				fIntersectVolumeWithSceneGraph<tObbf>( volume, sg, parent, proximity );
			}
			break;
		case cShapeSphere:
			{
				const tSpheref volume = fToSphere( xform );
				sync_event_v_c( Math::tVec4f( volume.mCenter, volume.mRadius ), tSync::cSCProximity );
				fIntersectVolumeWithSceneGraph<tSpheref>( volume, sg, parent, proximity );
			}
			break;
		default:
			sigassert( !"invalid shape" );
			break;
		}

		if( Debug_Proximity_Render )
			fDebugRender( sg, xform, proximity );
	}

	void tProximity::tShape::fDebugRender( const tSceneGraph& sg, const Math::tMat3f& xform, const tProximity& proximity ) const
	{
#ifdef sig_devmenu
		switch( mType )
		{
		case cShapeAabb:
			{
				const tAabbf volume = fToAabb( xform );
				sg.fDebugGeometry( ).fRenderOnce( volume, proximity.fDebugColor( ) );
			}
			break;
		case cShapeObb:
			{
				const tObbf volume = fToObb( xform );
				sg.fDebugGeometry( ).fRenderOnce( volume, proximity.fDebugColor( ) );
			}
			break;
		case cShapeSphere:
			{
				const tSpheref volume = fToSphere( xform );
				sg.fDebugGeometry( ).fRenderOnce( volume, proximity.fDebugColor( ) );
			}
			break;
		default:
			sigassert( !"invalid shape" );
			break;
		}
#endif//sig_devmenu
	}

	tProximity::tProximityGroupList tProximity::sGroups;
	b32	tProximity::sGroupsNeedCleaned = false;

	tProximity::tProximity( )
		: mPostFilterFunction( 0 )
		, mRefreshFrequency( 0.f )
		, mElapsedTime( 0.f )
		, mFilterByLogicEnts( false )
		, mTrackNewEnts( false )
		, mQueryInheritedProperties( false )
		, mPaused( false )
		, mRespectGroupMax( false )
	{
		if_devmenu( mDebugColor = tRandom::fSubjectiveRand( ).fColor( 0.25f ) );
	}
	tProximity::tProximity( const tStringPtr& groupName, u32 maxHitsPerFrame )
		: mPostFilterFunction( 0 )
		, mRefreshFrequency( 0.f )
		, mElapsedTime( 0.f )
		, mFilterByLogicEnts( false )
		, mTrackNewEnts( false )
		, mQueryInheritedProperties( false )
		, mPaused( false )
		, mGroupName( groupName )
		, mRespectGroupMax( false )
	{		
		tProximityGroup* group = sGroups.fFind( groupName );
		if( !group ) group = sGroups.fInsert( groupName, tProximityGroup( maxHitsPerFrame ) );
	}
	tProximity::~tProximity( )
	{
	}
	b32 tProximity::fRefreshMT( f32 dt, const tEntity& parent, const Math::tMat3f *explicitXform )
	{
		if( mPaused )
			return false;

		mElapsedTime -= dt;

		if( mElapsedTime <= 0.f )
		{
			if( mRespectGroupMax && !mGroupName.fNull( ) )
			{
				tProximityGroup* group = sGroups.fFind( mGroupName );
				sigassert( group ) ;
				if( ++group->mHits > group->mMaxHits )
				{
					log_warning( 0, "Did not refresh tProximity, too many this frame." );
					return false;	
				}

				sGroupsNeedCleaned = true;
			}

			fRefreshEntityList( parent, explicitXform );
			return true;
		}

		return false;
	}
	void tProximity::fDebugRender( const tSceneGraph& sg, const Math::tMat3f& xform ) const
	{
		for( u32 i = 0; i < mShapes.fCount( ); ++i )
			mShapes[ i ].fDebugRender( sg, xform, *this );
	}
	void tProximity::fCleanST( )
	{
		sync_event_v_c( mElapsedTime, tSync::cSCProximity );
		sync_event_v_c( fEntityCount( ), tSync::cSCProximity );
		sync_event_v_c( fNewEntityCount( ), tSync::cSCProximity );

		mNewEntsToDelete.fSetCount( 0 );
		mFilteredEntsToDelete.fSetCount( 0 );
		mLastFilteredEnts.fSetCount( 0 );

		if( sGroupsNeedCleaned )
		{
			sGroupsNeedCleaned = false;
			for( tProximityGroupList::tConstIteratorNoNullOrRemoved it( sGroups.fBegin( ), sGroups.fEnd( ) ); it != sGroups.fEnd( ); ++it )
				it->mValue.mHits = 0;
		}
	}
	void tProximity::fReleaseAllEntityRefsST( )
	{
		fCleanST( );
		mNewEnts.fSetCount( 0 );
		mFilteredEnts.fSetCount( 0 );
		mLastFilteredEnts.fSetCount( 0 );
	}
	void tProximity::fPause( b32 pause )
	{ 
		// If we are currently paused and are about to unpause then force a refresh of the entity list
		if( mPaused && !pause )
			mElapsedTime = 0.f;
		mPaused = pause;
	}
	void tProximity::fSetRefreshFrequency( f32 frequency, f32 freqRandDeviation )
	{
		mRefreshFrequency = frequency;
		if( freqRandDeviation > 0.f )
			mRefreshFrequency += sync_rand( fFloatInRange( -freqRandDeviation, +freqRandDeviation ) );
	}
	void tProximity::fAddAabb( const tAabbf& aabb )
	{
		sync_event_v_c( aabb, tSync::cSCProximity );
		mShapes.fPushBack( tShape( cShapeAabb, aabb ) );
	}
	void tProximity::fAddObb( const tAabbf& aabb )
	{
		sync_event_v_c( aabb.mMax, tSync::cSCProximity );
		sync_event_v_c( aabb.mMin, tSync::cSCProximity );
		mShapes.fPushBack( tShape( cShapeObb, aabb ) );
	}
	void tProximity::fAddSphere( const tSpheref& sphere )
	{
		sync_event_v_c( Math::tVec4f( sphere.mCenter, sphere.mRadius), tSync::cSCProximity );
		mShapes.fPushBack( tShape( cShapeSphere, tAabbf( sphere ) ) );
	}
	void tProximity::fAddEntity( tEntity* ent )
	{
		sigassert( ent );

		sync_event_v_c( ent->fGuid( ), tSync::cSCProximity );
		
		if( mFilterByLogicEnts )
		{
			tEntity* logicEnt = ent->fFirstAncestorWithLogic( );
			if( logicEnt )
			{
				if( mLogicFilter.fSet( ) )
				{
					if( mQueryInheritedProperties ) 
					{
						if( !tFilter_Inherited::fLogicFilter( mLogicFilter, logicEnt ) )
							return;
					}
					else if( !tFilter::fLogicFilter( mLogicFilter, logicEnt ) ) 
						return;
				}

				// always have to check this list when filtering by logic since many meshes man have the same logic parent.
				if( !mFilteredEnts.fFind( logicEnt ) )
				{
					mFilteredEnts.fPushBack( tEntityPtr( logicEnt ) );

					if( mTrackNewEnts && !mLastFilteredEnts.fFind( logicEnt ) )
						mNewEnts.fPushBack( tEntityPtr( logicEnt ) );
				}
			}
		}
		else
		{
			mFilteredEnts.fPushBack( tEntityPtr( ent ) );

			if( mTrackNewEnts && !mLastFilteredEnts.fFind( ent ) )
				mNewEnts.fPushBack( tEntityPtr( ent ) );
		}
	}
	void tProximity::fRefreshEntityList( const tEntity& parent, const Math::tMat3f *explicitXform )
	{
		mElapsedTime = mRefreshFrequency;

		sigassert( parent.fSceneGraph( ) );

		if( mTrackNewEnts )
		{
			if( mNewEntsToDelete.fCount( ) == 0 )
				mNewEnts.fSwap( mNewEntsToDelete );
			else
			{
				mNewEntsToDelete.fJoin( mNewEnts );
				mNewEnts.fSetCount( 0 );
			}

			sigassert( !mLastFilteredEnts.fCount( ) && "You must clean every frame!" );
			mLastFilteredEnts.fSwap( mFilteredEnts );
		}
		else
		{
			if( mFilteredEntsToDelete.fCount( ) == 0 )
				mFilteredEnts.fSwap( mFilteredEntsToDelete );
			else
			{
				mFilteredEntsToDelete.fJoin( mFilteredEnts );
				mFilteredEnts.fSetCount( 0 );
			}
		}

		const tSceneGraph& sg = *parent.fSceneGraph( );
		const Math::tMat3f& xform = explicitXform ? *explicitXform : parent.fObjectToWorld( );

		for( u32 i = 0; i < mShapes.fCount( ); ++i )
			mShapes[ i ].fIntersect( sg, parent, xform, *this );

		if( mPostFilterFunction )
			mPostFilterFunction( parent, *this, mFilteredEnts );
	}


	tMovingProximity::tMovingProximity( )
		: mLocalBounds( tAabbf::cZeroSized )
		, mWorldBounds( tObbf::cZeroSized )
		, mProxInitialized( false )
	{
	}

	tMovingProximity::tMovingProximity( const tStringPtr& groupName, u32 maxHitsPerFrame )
		: mLocalBounds( tAabbf::cZeroSized )
		, mWorldBounds( tObbf::cZeroSized )
		, mProxInitialized( false )
		, mProximity( groupName, maxHitsPerFrame )
	{

	}

	void tMovingProximity::fOnDelete( )
	{
		mProximity.fReleaseAllEntityRefsST( );
	}

	void tMovingProximity::fSetBounds( const tAabbf& bounds )
	{
		mLocalBounds = bounds;
	}

	void tMovingProximity::fUpdateIdeal( tLogic* logic, const tVec3f& zDir, const tVec3f& worldSpaceFutureOffset )
	{
		sigassert( !zDir.fIsNan( ) );

		const tMat3f& currentXform = logic->fOwnerEntity( )->fObjectToWorld( );
		sigassert( !currentXform.fIsNan( ) );

		tVec3f z = zDir;
		z.y = 0;
		f32 length = z.fLength( );
		if( fEqual( length, 0.f ) )
		{
			z = currentXform.fZAxis();
			z.fNormalizeSafe( tVec3f::cZAxis );
		}
		else
		{
			z /= length;
		}

		tMat3f xform;
		xform.fSetTranslation( currentXform.fGetTranslation( ) );
		xform.fOrientZAxis( z, tVec3f::cYAxis );


		for( u32 i = 0; i < mProbePoints.fCount( ); ++i )
		{
			if( !mWorldBounds.fContains( xform.fXformPoint( mProbePoints[ i ] ) + worldSpaceFutureOffset ) )
			{
				fMoveProximityMT( logic, xform );
				break;
			}
		}
	}

	void tMovingProximity::fUpdateCritical( tLogic* logic, const tVec3f& zDir )
	{
		if( mProximity.fRespectGroupMax( ) ) 
			return; //dont do any critical updates if we're respecting group max

		const tMat3f& currentXform = logic->fOwnerEntity( )->fObjectToWorld( );

		tVec3f z = zDir;
		z.y = 0;
		f32 length = z.fLength( );
		if( fEqual( length, 0.f ) )
		{
			z = currentXform.fZAxis();
			z.fNormalizeSafe( tVec3f::cZAxis );
		}
		else
		{
			z /= length;
		}

		tMat3f xform;
		xform.fSetTranslation( currentXform.fGetTranslation( ) );
		xform.fOrientZAxis( z, tVec3f::cYAxis );

		for( u32 i = 0; i < mProbePoints.fCount( ); ++i )
		{
			if( !mWorldBounds.fContains( xform.fXformPoint( mProbePoints[ i ] ) ) )
			{
				fMoveProximityMT( logic, xform );
				if( mProxInitialized )
					log_warning_nospam( 0, "Had to update tMovingProximity in Critical!" );
				break;
			}
		}
	}

	void tMovingProximity::fCleanST( )
	{
		mProximity.fCleanST( );
	}

	void tMovingProximity::fMoveProximityMT( tLogic* logic, const tMat3f& currentXform )
	{
		mWorldBounds = tObbf( mLocalBounds, currentXform );

		mProximity.fClearShapes( );
		mProximity.fAddObb( mLocalBounds );
		mProximity.fRefreshMT( 0.f, *logic->fOwnerEntity( ), &currentXform );

		mProxInitialized = true;
	}

}

namespace Sig
{
	namespace
	{
		static void fAddProperty( tProximity::tPropertyGroup* group, u32 key, u32 value )
		{
			group->fAddProperty( tEntityEnumProperty( key, value ) );
		}

		static void fRemoveProperty( tProximity::tPropertyGroup* group, u32 key, u32 value )
		{
			group->fRemoveProperty( tEntityEnumProperty( key, value ) );
		}

		static void fAddMustHaveProperty( tProximity::tPropertyGroup* group, u32 key, u32 value )
		{
			group->fAddMustHaveProperty( tEntityEnumProperty( key, value ) );
		}

		static void fRemoveMustHaveProperty( tProximity::tPropertyGroup* group, u32 key, u32 value )
		{
			group->fRemoveMustHaveProperty( tEntityEnumProperty( key, value ) );
		}
	}


	void tProximity::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tProximity::tPropertyGroup, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("AddTag"),						&tPropertyGroup::fAddTag)
				.Func(_SC("RemoveTag"),						&tPropertyGroup::fRemoveTag)
				.Prop(_SC("Tags"),							&tPropertyGroup::fTags, &tPropertyGroup::fSetTags)
				.GlobalFunc(_SC("AddProperty"),				&fAddProperty)
				.GlobalFunc(_SC("RemoveProperty"),			&fRemoveProperty)	
				.GlobalFunc(_SC("AddMustHaveProperty"),		&fAddMustHaveProperty)
				.GlobalFunc(_SC("RemoveMustHaveProperty"),	&fRemoveMustHaveProperty)			
				;
			vm.fRootTable( ).Bind(_SC("ProximityPropertyGroup"), classDesc);
		}
		{
			Sqrat::Class<tProximity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("ClearShapes"),					&tProximity::fClearShapes)
				.Func(_SC("AddAabb"),						&tProximity::fAddAabb)
				.Func(_SC("AddObb"),						&tProximity::fAddObb)
				.Func(_SC("AddSphere"),						&tProximity::fAddSphere)
				.Func(_SC("EntityCount"),					&tProximity::fEntityCount)
				.Func(_SC("GetEntity"),						&tProximity::fGetEntity)
				.Func(_SC("Pause"),							&tProximity::fPause)
				.Func(_SC("SetRefreshFrequency"),				&tProximity::fSetRefreshFrequency)
				.Func(_SC("FilterByLogicEnts"),				&tProximity::fFilterByLogicEnts)
				.Func(_SC("SetFilterByLogicEnts"),			&tProximity::fSetFilterByLogicEnts)
				.Func(_SC("QueryInheritedProperties"),		&tProximity::fQueryInheritedProperties)
				.Func(_SC("SetQueryInheritedProperties"),	&tProximity::fSetQueryInheritedProperties)
				.Prop(_SC("Filter"),						&tProximity::fFilterForScript)
				.Prop(_SC("LogicFilter"),					&tProximity::fLogicFilterForScript)
				.Prop(_SC("TrackNewEnts"),					&tProximity::fTrackNewEnts, &tProximity::fSetTrackNewEnts)
				;
			vm.fRootTable( ).Bind(_SC("Proximity"), classDesc);
		}
	}
}
