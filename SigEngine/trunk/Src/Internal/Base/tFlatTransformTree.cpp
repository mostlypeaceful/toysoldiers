#include "BasePch.hpp"
#include "tFlatTransformTree.hpp"

using namespace Sig::Math;

namespace Sig
{

	Math::tMat3f& tFlatTransTreeRef::fParentRelXform( )
	{ 
		sigassert( mTree ); 
		return mTree->fParentRelXform( *this ); 
	}

	const Math::tMat3f& tFlatTransTreeRef::fParentRelXform( ) const
	{ 
		sigassert( mTree ); 
		return mTree->fParentRelXform( *this ); 
	}

	const Math::tMat3f& tFlatTransTreeRef::fWorldXform( ) const		
	{ 
		sigassert( mTree ); 
		return mTree->fWorldXform( *this ); 
	}

	const Math::tMat3f& tFlatTransTreeRef::fWorldXformInv( ) const	
	{ 
		sigassert( mTree ); 
		return mTree->fWorldXformInv( *this ); 
	}

	Math::tAabbf& tFlatTransTreeRef::fLocalBounds( )				
	{ 
		sigassert( mTree ); 
		return mTree->fLocalBounds( *this ); 
	}

	const Math::tAabbf& tFlatTransTreeRef::fWorldBounds( ) const 
	{ 
		sigassert( mTree ); 
		return mTree->fWorldBounds( *this ); 
	}


	void tFlatTransformTree::tLayer::fInsert( tFlatTransTreeRef& item, u32 childStart, const Math::tMat3f& parentRel, const Math::tAabbf& localBounds )
	{
		u32 index = item.mIndex;
		mChildStart.fInsert( index, childStart );
		mChildCount.fInsert( index, 0u );
		mParentRel.fInsert( index, parentRel );
		mWorldXform.fInsert( index, tMat3f::cIdentity );
		mWorldXformInv.fInsert( index, tMat3f::cIdentity );
		mLocalBounds.fInsert( index, localBounds );
		mWorldBounds.fInsert( index, tAabbf::cZeroSized );
		mItems.fInsert( index, &item );
	}

	void tFlatTransformTree::tLayer::fRemove( tFlatTransTreeRef& item )
	{
		u32 index = item.mIndex;
		mChildStart.fEraseOrdered( index );
		mChildCount.fEraseOrdered( index );
		mParentRel.fEraseOrdered( index );
		mWorldXform.fEraseOrdered( index );
		mWorldXformInv.fEraseOrdered( index );
		mLocalBounds.fEraseOrdered( index );
		mWorldBounds.fEraseOrdered( index );
		mItems.fEraseOrdered( index );

		item.fInvalidate( );
	}

	tFlatTransformTree::tFlatTransformTree( )
	{
		// setup root
		mRoot.mLayer = 0;
		mRoot.mIndex = 0;
		mRoot.mTree = this;

		mLayers.fPushBack( tLayer( ) );
		tLayer& layer = mLayers[ 0 ];

		layer.fInsert( mRoot, 0, tMat3f::cIdentity, tAabbf::cZeroSized );
	}

	tFlatTransformTree::~tFlatTransformTree( )
	{
		mRoot.fInvalidate( );
	}

	void tFlatTransformTree::fInsert( const tFlatTransTreeRef& parent, tFlatTransTreeRef& me, const Math::tMat3f& parentRel, const Math::tAabbf& localBounds )
	{
		sigassert( !me.fInTree( ) );
		sigassert( parent.fInTree( ) );
		sigassert( me != mRoot );

		tLayer& parentLayer = mLayers[ parent.mLayer ];

		const u32 layerID = parent.mLayer + 1;
		const u32 index = parentLayer.mChildStart[ parent.mIndex ] + parentLayer.mChildCount[ parent.mIndex ];

		// update parent layer
		++parentLayer.mChildCount[ parent.mIndex ];
		for( u32 i = parent.mIndex + 1; i < parentLayer.fCount( ); ++i )
			++parentLayer.mChildStart[ i ];

		// see if we need a new layer
		if( layerID == mLayers.fCount( ) )
			mLayers.fPushBack( tLayer( ) );

		tLayer& layer = mLayers[ layerID ];

		// update our layer
		for( u32 i = index; i < layer.fCount( ); ++i )
			++layer.mItems[ i ]->mIndex;

		// determine our child's starting index
		u32 childrenStart = 0;
		if( index != 0 )
		{
			u32 prevSib = index - 1;
			childrenStart = layer.mChildStart[ prevSib ] + layer.mChildCount[ prevSib ];
		}

		// add data
		me.mIndex = index;
		me.mLayer = layerID;
		me.mTree = this;
		layer.fInsert( me, childrenStart, parentRel, localBounds );
	}

	void tFlatTransformTree::fRemove( const tFlatTransTreeRef& parent, tFlatTransTreeRef& me )
	{
		sigassert( me.fInTree( ) );
		sigassert( parent.fInTree( ) );
		sigassert( me != mRoot );

		tLayer& parentLayer = mLayers[ parent.mLayer ];

		// update parent layer
		--parentLayer.mChildCount[ parent.mIndex ];
		for( u32 i = parent.mIndex + 1; i < parentLayer.fCount( ); ++i )
			--parentLayer.mChildStart[ i ];

		// update our layer
		const u32 myIndex = me.mIndex;
		tLayer& layer = mLayers[ me.mLayer ];
		sigassert( layer.mChildCount[ myIndex ] == 0 );

		for( u32 i = myIndex + 1; i < layer.fCount( ); ++i )
			--layer.mItems[ i ]->mIndex;

		layer.fRemove( me );
	}

	void tFlatTransformTree::fUpdate( )
	{
		fUpdateTransforms( );
	}

	void tFlatTransformTree::fUpdateTransforms( )
	{
		for( u32 l = 0; l < mLayers.fCount( ) - 1; ++l )
		{
			const tLayer& parentLayer = mLayers[ l ];
			tLayer& childLayer = mLayers[ l + 1 ];

			const u32*		childCount = parentLayer.mChildCount.fBegin( );
			const tMat3f*	parentXform = parentLayer.mWorldXform.fBegin( );
			tMat3f*			childXform = childLayer.mWorldXform.fBegin( );
			tMat3f*			childXformInv = childLayer.mWorldXformInv.fBegin( );
			tMat3f*			childRelXform = childLayer.mParentRel.fBegin( );
			tAabbf*			childBounds = childLayer.mLocalBounds.fBegin( );
			tAabbf*			childWBounds = childLayer.mLocalBounds.fBegin( );

			for( ; parentXform != parentLayer.mWorldXform.fEnd( ); ++parentXform, ++childCount )
			{
				tMat3f* childEnd = childXform + *childCount;
				for( ; childXform != childEnd; ++childXform, ++childXformInv, ++childRelXform, ++childBounds, ++childWBounds )
				{
					*childXform = *parentXform * *childRelXform;
					*childXformInv = childXform->fInverse( );
					*childWBounds = childBounds->fTransform( *childXform );
				}
			}
		}
	}
}
