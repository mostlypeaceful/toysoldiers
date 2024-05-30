 #ifndef __tCollisionShapes__
#define __tCollisionShapes__

#include "tSortedOverlapTree.hpp"
#include "Math/tConvexHull.hpp"

namespace Sig { 

	class tShapeEntity;

namespace Physics
{

	class tPhysicsBody;
	class tPhysicsWorld;

	enum tCollisionShapes
	{
		cCollisionShapeOBB,
		cCollisionShapeSphere,
		cCollisionShapeCapsule,
		cCollisionShapeCylinder,
		cCollisionShapeHeightField,
		cCollisionShapeMesh,
		cCollisionShapeConvexHull,
		cCollisionShapeRay,
		cCollisionShapeCount
	};

	class tCollisionShape : public tRefCounter
	{
	public:
		// This will return the shape in the shape entity's parent's space
		static tCollisionShape* fFromShapeEntity( tShapeEntity* se, const Math::tMat3f& parentInvXform );
		static void fUpdateTransform( tCollisionShape* cs, tShapeEntity* se, const Math::tMat3f& parentInvXform );

		static u32 fMakeKey( const tCollisionShape& a, const tCollisionShape& b );

		// When the extra radius has been subtracted, axes will be clamped to this min size.
		static const f32 cMinimumAxisHalfSize;

		template< class t >
		t& fCast( ) 
		{
			sigassert( fType( ) == t::cShapeType );
			return static_cast<t&>( *this );
		}

		enum tFlags
		{
			cPassive	= (1<<0), //does not react to collision events.
			cVolatile	= (1<<1), //shape is expected to change continuously.
		};

	public:
		tCollisionShape( u8 type = cCollisionShapeCount, f32 extraRadius = 0.f );
		virtual ~tCollisionShape( ) { }

		const u8& fType( ) const { return mType; }

		tPhysicsBody* fOwner( ) const { return mOwner; }
		void fRemoveFromOwner( );

		tEntityPtr&			fUserData( ) { return mUserData; }
		const tEntityPtr&	fUserData( ) const { return mUserData; }

		// Change the objects location by some xform in owner space
		virtual void		fTransform( const Math::tMat3f& xform ) { }
		virtual void		fShapeChanged( ) { }

		// Includes owner's xform
		const Math::tMat3f& fWorldXform( ) const;
		const Math::tAabbf& fCachedWorldAABB( ) const { sigassert( mItem ); return mItem->mBounds; }
		
		virtual Math::tAabbf fLocalAABB( );
		virtual Math::tAabbf fWorldAABB( );

		Math::tAabbf fWorldAABB( const Math::tVec3f& deltaFromV ) 
		{ 
			Math::tAabbf p0 = fWorldAABB( );
			p0 |= p0.fTranslate( deltaFromV );
			return p0;
		}

		b32  fFlagSet( u32 flag ) const { return fTestBits( mFlags, flag ); }
		void fSetFlag( u32 flag, b32 set ) { mFlags = fSetClearBits( mFlags, flag, set ); }

		// Identity Mask is a bit field representing which collision groups the shapes belong to. (Such as player, or debris layers.)
		u32 fIdentityMask( ) const { return mIdentityMask; }
		void fSetIdentityMask( u32 mask ) { mIdentityMask = mask; }

		// Collision Mask is a bit field representing which collision groups this shape should collide with. (Such as player, or debris layers.)
		u32 fCollisionMask( ) const { return mCollisionMask; }
		void fSetCollisionMask( u32 mask ) { mCollisionMask = mask; }

		b32 fCollisionMaskAgainst( const tCollisionShape& other ) const
		{
			return (((fIdentityMask( ) & other.fCollisionMask( )) != 0)
				&& ((other.fIdentityMask( ) & fCollisionMask( )) != 0));
		}

	public:
		// ################### Dont call any of this stuff, internal use only #####################
		void fAddToTree( tSortedOverlapTree& tree );

		void fAddToBatch( tSortedOverlapTree::tBatch& batch );

		void fRemoveFromTree( tSortedOverlapTree& tree )
		{
			sigassert( mItem );
			tree.fRemoveItem( *mItem );
			mItem.fRelease( );
		}

		void fUpdateTree( tSortedOverlapTree& tree )
		{
			sigassert( mItem );
			tree.fUpdateItem( *mItem, fWorldAABB( ) );
		}

		void fClearPairData( tSortedOverlapTree& tree )
		{
			sigassert( mItem );
			tree.fClearPairData( *mItem );
		}

		void fUpdateTree( tSortedOverlapTree& tree, const Math::tVec3f& deltaFromV )
		{
			sigassert( mItem );
			tree.fUpdateItem( *mItem, fWorldAABB( deltaFromV ) );
		}

		void fSleep( tSortedOverlapTree& tree, b32 sleep )
		{
			sigassert( mItem );
			tree.fSetSleeping( *mItem, sleep );
		}

		virtual void fDebugDraw( ) 
		{ }

		u32 fComputeFlags( ) const;

		tPhysicsBody*				mOwner;
		f32							mExtraRadius;
		f32							mExtraBroadphase;
		static const Math::tVec4f	cDebugColor; //static to save space, make it a member if you need Hardcore Debugging (tm)
		// ########################################################################################

	private:
		u8 mType;
		u8 mFlags;
		u32 mIdentityMask;
		u32 mCollisionMask;
		tSortedOverlapTree::tItemPtr mItem;
		tEntityPtr mUserData;
	};

	typedef tRefCounterPtr<tCollisionShape> tCollisionShapePtr;

	struct tCollisionShapeOBB : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeOBB;
		Math::tObbf mShape;

		tCollisionShapeOBB( const Math::tObbf& obb );

		virtual Math::tAabbf fLocalAABB( );
		virtual Math::tAabbf fWorldAABB( );

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mShape = mShape.fTransform( xform );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeSphere : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeSphere;
		Math::tVec3f mCenter;

		tCollisionShapeSphere( const Math::tSpheref& sphere )
			: tCollisionShape( cShapeType, sphere.fRadius( ) )
			, mCenter( sphere.fCenter( ) )
		{ }

		virtual Math::tAabbf fLocalAABB( );
		virtual Math::tAabbf fWorldAABB( );

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mCenter = xform.fXformPoint( mCenter );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeCapsule : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeCapsule;
		Math::tCapsule mShape;

		tCollisionShapeCapsule( const Math::tCapsule& capsule );

		virtual Math::tAabbf fLocalAABB( );
		virtual Math::tAabbf fWorldAABB( );

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mShape = mShape.fTransform( xform );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeCylinder : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeCylinder;
		Math::tCylinder mShape;
		Math::tAabbf mLocalAABB;

		tCollisionShapeCylinder( const Math::tCylinder& cylinder );

		virtual void fShapeChanged( );
		virtual Math::tAabbf fLocalAABB( ) { return mLocalAABB; }
		virtual Math::tAabbf fWorldAABB( ) { return mLocalAABB.fTransform( fWorldXform( ) ); }

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mShape = mShape.fTransform( xform );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeHeightfield : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeHeightField;
		tEntityPtr mHeightField;
		Math::tAabbf mLocalAABB;

		tCollisionShapeHeightfield( const tEntity& heightField );

		virtual void fShapeChanged( );
		virtual Math::tAabbf fLocalAABB( ) { return mLocalAABB; }
		virtual Math::tAabbf fWorldAABB( ) { return mLocalAABB; }

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			sigassert( !"transforming not supported on heightfields. user data referenced." );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeMesh : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeMesh;
		tEntityPtr mMesh;
		Math::tAabbf mLocalAABB;

		tCollisionShapeMesh( const tEntity& meshEntity );

		virtual void fShapeChanged( );
		virtual Math::tAabbf fLocalAABB( ) { return mLocalAABB; }
		virtual Math::tAabbf fWorldAABB( ) { return mLocalAABB; }

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			sigassert( !"transforming not supported on meshes. user data referenced." );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeConvexHull : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeConvexHull;
		const Math::tConvexHull* mShape;
		Math::tMat3f mLocalXform; //must update local bounds if you update this
		Math::tAabbf mLocalAABB;

		tCollisionShapeConvexHull( const Math::tConvexHull* hull, const Math::tMat3f& localXform );

		virtual void fShapeChanged( );
		virtual Math::tAabbf fLocalAABB( ) { return mLocalAABB; }
		virtual Math::tAabbf fWorldAABB( ) { return mLocalAABB.fTransform( fWorldXform( ) ); }

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mLocalXform = xform * mLocalXform;
			fShapeChanged( );
		}

		virtual void fDebugDraw( );
	};

	struct tCollisionShapeRay : public tCollisionShape
	{
		static const u32 cShapeType = cCollisionShapeRay;
		Math::tRayf mShape;
		Math::tAabbf mLocalAABB;

		tCollisionShapeRay( const Math::tRayf& ray, f32 broadphaseBoundary );

		virtual Math::tAabbf fLocalAABB( ) { return mLocalAABB; }
		virtual Math::tAabbf fWorldAABB( ) { return mLocalAABB.fTransform( fWorldXform( ) ); }

		virtual void fTransform( const Math::tMat3f& xform ) 
		{
			mShape = mShape.fTransform( xform );
		}

		virtual void fDebugDraw( );
	};
	
	class tCollisionDispatch
	{
	public:
		declare_singleton_define_own_ctor_dtor( tCollisionDispatch );

		tCollisionDispatch( ) { fRegisterAllColliders( ); }
		~tCollisionDispatch( ) { }

		class tAgent : public tSortedOverlapTree::tPairData
		{
		public: 
			b32 mVolatile;

			tAgent( tCollisionShape& a, tCollisionShape& b );

			virtual void fStepMT( f32 dt ) { }
			virtual void fStepST( f32 dt ) { }

			//update your support mappings and things, shapes have changed relative to bodies.
			virtual void fVolatileUpdate( ) { }
		};

		typedef tRefCounterPtr< tAgent > tAgentPtr;

		typedef tAgent* (*tCreateAgentFuncPtr)( tCollisionShape& shapeA, tCollisionShape& shapeB );

		// will automatically register the reverse.
		void fRegisterCollider( tCreateAgentFuncPtr func, u32 typeA, u32 typeB );

		// Does simple bit masking, looks into the Collider map and creates the appropriate collision agent.
		tAgent* fCreateAgent( tCollisionShape& a, tCollisionShape& b );

	public:
		struct tCreateAgentFunc
		{
			tCreateAgentFuncPtr	mFunc;
			b32					mFlip;

			tCreateAgentFunc( tCreateAgentFuncPtr func = NULL, b32 flip = false )
				: mFunc( func )
				, mFlip( flip )
			{ }
		};

		tFixedArray< tFixedArray< tCreateAgentFunc, cCollisionShapeCount >, cCollisionShapeCount > mColliders;
		void fRegisterAllColliders( );
	};

}}

#endif//__tCollisionShapes__
