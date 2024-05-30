#ifndef __tFlatTransformTree__
#define __tFlatTransformTree__

namespace Sig
{
	class tFlatTransformTree;

	struct tFlatTransTreeRef
	{
		tFlatTransformTree* mTree;
		u32 mLayer;
		u32 mIndex;
		Math::tMat3f mPreSpawnXform;

		tFlatTransTreeRef( )
			: mTree( NULL )
			, mLayer( ~0 )
			, mIndex( ~0 )
			, mPreSpawnXform( Math::tMat3f::cIdentity )
		{ }

		~tFlatTransTreeRef( )
		{
			sigassert( !fInTree( ) );
		}

		b32 operator != ( const tFlatTransTreeRef& other ) { return mLayer != other.mLayer || mIndex != other.mIndex; }

		inline b32  fInTree( ) const	{ return mTree != NULL; }
		inline void fInvalidate( )		{ mTree = NULL; mLayer = ~0; mIndex = ~0; }
		
		Math::tMat3f&		fParentRelXform( );
		const Math::tMat3f& fParentRelXform( ) const;
		const Math::tMat3f& fWorldXform( ) const;
		const Math::tMat3f& fWorldXformInv( ) const;
		Math::tAabbf&		fLocalBounds( );
		const Math::tAabbf& fWorldBounds( ) const;

	};

	class tFlatTransformTree : public tRefCounter
	{
	public:
		tFlatTransformTree( );
		~tFlatTransformTree( );

		const tFlatTransTreeRef& fRoot( ) const { return mRoot; }
		
		// "me" must not be destroyed while it is in the tree!
		void fInsert( const tFlatTransTreeRef& parent, tFlatTransTreeRef& me, const Math::tMat3f& parentRel, const Math::tAabbf& localBounds );
		void fRemove( const tFlatTransTreeRef& parent, tFlatTransTreeRef& me );

		void fUpdate( );

		Math::tMat3f& fParentRelXform( const tFlatTransTreeRef& item )				{ return mLayers[ item.mLayer ].mParentRel[ item.mIndex ]; }
		const Math::tMat3f& fParentRelXform( const tFlatTransTreeRef& item ) const	{ return mLayers[ item.mLayer ].mParentRel[ item.mIndex ]; }
		const Math::tMat3f& fWorldXform( const tFlatTransTreeRef& item ) const		{ return mLayers[ item.mLayer ].mWorldXform[ item.mIndex ]; }
		const Math::tMat3f& fWorldXformInv( const tFlatTransTreeRef& item ) const	{ return mLayers[ item.mLayer ].mWorldXformInv[ item.mIndex ]; }

		Math::tAabbf& fLocalBounds( const tFlatTransTreeRef& item )				{ return mLayers[ item.mLayer ].mLocalBounds[ item.mIndex ]; }
		const Math::tAabbf& fWorldBounds( const tFlatTransTreeRef& item ) const { return mLayers[ item.mLayer ].mWorldBounds[ item.mIndex ]; }

	private:
		struct tLayer
		{
			tGrowableArray< u32 >			mChildStart;
			tGrowableArray< u32 >			mChildCount;
			tGrowableArray< Math::tMat3f >	mParentRel;
			tGrowableArray< Math::tMat3f >	mWorldXform;
			tGrowableArray< Math::tMat3f >	mWorldXformInv;
			tGrowableArray< Math::tAabbf >	mLocalBounds;
			tGrowableArray< Math::tAabbf >	mWorldBounds;
			tGrowableArray< tFlatTransTreeRef* > mItems;

			u32 fCount( ) const { return mItems.fCount( ); }
			void fInsert( tFlatTransTreeRef& item, u32 childStart, const Math::tMat3f& parentRel, const Math::tAabbf& localBounds );
			void fRemove( tFlatTransTreeRef& item );
		};

		tFlatTransTreeRef			mRoot;
		tGrowableArray< tLayer >	mLayers;

		void fUpdateTransforms( );
	};

}

#endif//__tFlatTransformTree__
