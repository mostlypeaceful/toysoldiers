#include "BasePch.hpp"
#include "tPotentialVisibilitySet.hpp"
#include "tRenderableEntity.hpp"
#include "Math/tConvexHull.hpp"
#include "tCamera.hpp"
#include "tShapeEntity.hpp"

namespace Sig { namespace Gfx
{


	tPotentialVisibilitySet::tHullItem::tHullItem( tShapeEntity* shape, b32 inverted )
		: mShape( shape )
		, mInvered( inverted )
	{ }

	tPotentialVisibilitySet::tHullItem::~tHullItem( )
	{ }

	void tPotentialVisibilitySet::fRegister( tEntity& ent, const tVisibilitySetRef& set )
	{
		for( u32 i = 0; i < set.mSet.fCount( ); ++i )
		{
			tLayer* layer = fLayer( set.mSet[ i ], true );
			sigcheckfail( layer, continue );

			layer->mEntites.fFindOrAdd( tEntityPtr( &ent ) );
			fApplyVisibility( ent, layer->mVisible );
		}
	}

	void tPotentialVisibilitySet::fUnRegister( tEntity& ent, const tVisibilitySetRef& set )
	{
		for( u32 i = 0; i < set.mSet.fCount( ); ++i )
		{
			tLayer* layer = fLayer( set.mSet[ i ], false );
			sigcheckfail( layer, continue );
			
			const b32 found = layer->mEntites.fFindAndErase( &ent );
			sigassert( found && "Attempt to remove unknown entity from visibility set" );
		}
	}

	void tPotentialVisibilitySet::fSetVisibility( const tStringPtr& name, b32 visible )
	{
		tLayer* layer = fLayer( name, false );
		sigcheckfail( layer && "No layer found by that name.", return );

		fSetVisibility( *layer, visible );
	}


	void tPotentialVisibilitySet::fSetVisibility( tLayer& layer, b32 visible )
	{
		if( layer.mVisible != visible )
		{
			layer.mVisible = visible;

			for( u32 i = 0; i < layer.mEntites.fCount( ); ++i )
				fApplyVisibility( *layer.mEntites[ i ], visible );
		}
	}

	void tPotentialVisibilitySet::fClear( )
	{
		mLayers.fSetCount( 0 );
	}

	void tPotentialVisibilitySet::fUpdateForCamera( const Gfx::tCamera& camera )
	{
		for( u32 i = 0; i < mLayers.fCount( ); ++i )
		{
			tLayer& layer = *mLayers[ i ];

			u32 visible = 0;

			for( u32 h = 0; h < layer.mHulls.fCount( ); ++h )
			{
				b32 show = layer.mHulls[ h ].mShape->fContains( camera.fGetTripod( ).mEye );

				if( layer.mHulls[ h ].mInvered )
					show = !show;

				if( show )
					++visible;
			}

			fSetVisibility( layer, (visible > 0) );
		}
	}

	void tPotentialVisibilitySet::fRegisterConvexHull( tShapeEntity* shape, const tStringPtr& name, b32 inverted )
	{
		tLayer* layer = fLayer( name, true );
		sigcheckfail( layer, return );

		if( !layer->mHulls.fFind( shape ) )
			layer->mHulls.fPushBack( tHullItem( shape, inverted ) );
	}
	
	tPotentialVisibilitySet::tLayer* tPotentialVisibilitySet::fLayer( const tStringPtr& name, b32 createIfNotFound )
	{
		for( u32 i = 0; i < mLayers.fCount( ); ++i )
			if( mLayers[ i ]->mName == name )
				return mLayers[ i ].fGetRawPtr( );

		// not found
		if( createIfNotFound )
		{
			// add it.
			mLayers.fPushBack( tLayerPtr( NEW tLayer( name ) ) );
			return mLayers.fBack( ).fGetRawPtr( );
		}

		return NULL;
	}

	void tPotentialVisibilitySet::fApplyVisibility( tEntity& root, b32 visible )
	{
		Gfx::tRenderableEntity::fSetNotPotentiallyVisibleRecursive( root, !visible );
	}

}}

