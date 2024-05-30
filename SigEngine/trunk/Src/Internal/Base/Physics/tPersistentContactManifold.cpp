#include "BasePch.hpp"
#include "tPersistentContactManifold.hpp"
#include "tContactPoint.hpp"
#include "tPhysicsWorld.hpp"
#include "tRigidBody.hpp"
#include "tContactIsland.hpp"

using namespace Sig::Math;


namespace Sig { namespace Physics
{

	devvar( f32, Physics_RigidBody_Contact_KeepHorizSep, 0.1f );
	devvar( f32, Physics_RigidBody_Contact_KeepNormalSep, 0.1f );
	devvar( u32, Physics_RigidBody_Contact_StaticLife, 3 );
	devvar( bool, Physics_RigidBody_Contact_Speculate, true );
	//devvar( bool, Physics_RigidBody_Contact_SpeculateFull, false );
	devvar( f32, Physics_RigidBody_Contact_BaumgarteK, 0.4f );
	devvar( f32, Physics_RigidBody_Contact_BaumgarteS, 0.02f );

	devvar( bool, Physics_Debug_RenderContacts, false );
	devvar( bool, Physics_Debug_RenderNewContacts, false );	

	u32 tPersistentContactPt::sNextID = 0;


	tPersistentContactPt::tPersistentContactPt( tRigidBody& a, const tContactPoint& cp, tRigidBody* b )
		: mID( sNextID++ )
		, mLife( Physics_RigidBody_Contact_StaticLife )
		, mLastImpulse( 0.f )
		, mLastImpulseTangent( 0.f )
		, mLastImpulseTangentWorld( 0.f )
	{
		if( Physics_Debug_RenderNewContacts )
			cp.fRender( );

		mARel = a.fInvTransform( ).fXformPoint( cp.mPoint - cp.mNormal * cp.mDepth );
		mBRel = cp.mPoint;
		mBNormal = cp.mNormal;
		mWorldNormal = cp.mNormal;
		mDepth = cp.mDepth;
		mHorizSepLenSqr = 0.f;

		if( b )
		{
			mLife = 0; //dont keep dynamic contacts around longer than they exist, or things will get weird.
			mBRel = b->fInvTransform( ).fXformPoint( mBRel );
			mBNormal = b->fInvTransform( ).fXformVector( mWorldNormal );
		}
	}

	b32 tPersistentContactPt::fSimilar( const tPersistentContactPt& other ) const
	{		
		if( mBNormal.fDot( other.mBNormal ) < 0.75f )
			return false;

		const f32 cDistThresh = Physics_RigidBody_Contact_KeepHorizSep;
		const f32 cDistThreshSqrd = cDistThresh * cDistThresh;

		f32 dist = (mARel - other.mARel).fLengthSquared( );
		if( dist > cDistThreshSqrd )
			return false;

		dist = (mBRel - other.mBRel).fLengthSquared( );
		if( dist > cDistThreshSqrd )
			return false;

		return true;
	}

	void tPersistentContactPt::fUpdate( const tPersistentContactPt& newPt )
	{
		sigassert( fSimilar( newPt ) );

		mARel = newPt.mARel;
		mBRel = newPt.mBRel;
		mBNormal = newPt.mBNormal;
		mWorldNormal = newPt.mWorldNormal;
		mDepth = newPt.mDepth;
		mLife = newPt.mLife;
		mHorizSepLenSqr = newPt.mHorizSepLenSqr;
	}

	void tPersistentContactPt::fRecompute( tRigidBody& a, tRigidBody* b )
	{
		tVec3f bPt = fBWorldPt( b );
		tVec3f aPt = fAWorldPt( a );
		mWorldNormal = b ? b->fTransform( ).fXformVector( mBNormal ) : mBNormal;

		tVec3f sep = bPt - aPt;
		mDepth = mWorldNormal.fDot( sep );

		sep -= mWorldNormal * mDepth;
		mHorizSepLenSqr = sep.fLengthSquared( );
	}

	b32 tPersistentContactPt::fInvalid( tRigidBody& a )
	{
		const f32 cDistThreshSqrd = Physics_RigidBody_Contact_KeepHorizSep * Physics_RigidBody_Contact_KeepHorizSep;

		if( ( mHorizSepLenSqr > cDistThreshSqrd || mDepth < -Physics_RigidBody_Contact_KeepNormalSep ) )
		{
			if( mLife == 0 )
				return true;

			--mLife;
		}

		return false;
	}

	tVec3f tPersistentContactPt::fAWorldPt( const tRigidBody& a ) const
	{
		return a.fTransform( ).fXformPoint( mARel );
	}

	tVec3f tPersistentContactPt::fBWorldPt( const tRigidBody* b ) const
	{
		return b ? b->fTransform( ).fXformPoint( mBRel ) : mBRel;
	}

	tContactPoint tPersistentContactPt::fAContactPt( const tRigidBody& a ) const
	{
		tContactPoint cp( fAWorldPt( a ), -mWorldNormal, mDepth );
		return cp;
	}

	tContactPoint tPersistentContactPt::fBContactPt( const tRigidBody* b ) const
	{
		tContactPoint cp( fBWorldPt( b ), mWorldNormal, mDepth );
		return cp;
	}

	tPersistentContactManifold::tPersistentContactManifold( tRigidBody* a, tRigidBody* b, tCollisionShape& sa, tCollisionShape& sb, b32 flipped, u32 key )
		: mA( a )
		, mB( b )
		, mAShape( sa )
		, mBShape( sb )
		, mFlipped( flipped )
		, mUnlimited( false )
		, mPassive( false )
		, mKey( key )
	{
		if( mB )
			tContactIsland::fBeginContact( *mA, *mB, (u32)this );
	}

	tPersistentContactManifold::~tPersistentContactManifold( )
	{
		if( mB )
			tContactIsland::fEndContact( *mA, *mB, (u32)this );
	}

	void tPersistentContactManifold::fAddContact( const tContactPoint& cp )
	{
		if( !Physics_RigidBody_Contact_Speculate && cp.mDepth < 0.f )
			return;			

		tContactPoint cp2 = cp;
		if( mFlipped )
			cp2.fFlip( );

		tPersistentContactPt pt( *mA, cp2, mB );

		// see if it's a resting contact
		for( u32 i = 0; i < mColliding.fCount( ); ++i )
		{
			tPersistentContactPt& oldContact = mColliding[ i ];
			if( oldContact.fSimilar( pt ) )
			{
				oldContact.fUpdate( pt );
				return;
			}
		}

		if( mColliding.fCount( ) == 4 && !mUnlimited )
		{
			u32 index = fComputeNewPtIndex( pt );
			mColliding[ index ] = pt;
		}
		else
			mColliding.fPushBack( pt );
	}

	void tPersistentContactManifold::fRemoveOldContacts( )
	{
		// good place to render them also
		if( Physics_Debug_RenderContacts )
		{
			for( u32 c = 0; c < mColliding.fCount( ); ++c )
				mColliding[ c ].fBContactPt( mB ).fRender( tVec4f( 1,0,0,1 ) );
		}

		// clear dead contacts
		if( mPassive )
			mColliding.fSetCount( 0 );
		else
		{
			for( s32 i = mColliding.fCount( ) - 1; i >= 0; --i )
			{
				tPersistentContactPt& oldContact = mColliding[ i ];
				if( oldContact.fInvalid( *mA ) )
					mColliding.fErase( i );
			}
		}
	}

	namespace
	{
		f32 fCPSArea( const tVec3f& a, u32 b, u32 c, u32 d, const tPersistentContactManifold::tPtArray& data )
		{
			return (data[b].mBRel-a).fCross( data[c].mBRel-data[d].mBRel ).fLengthSquared( );
		}
	}

	// only happens where there are 4 pts.
	u32 tPersistentContactManifold::fComputeNewPtIndex( const tPersistentContactPt& pt ) const
	{
		u32 deepest = ~0;
		f32 depth = pt.mDepth;

		// find deepest
		for( u32 i = 0; i < mColliding.fCount( ); ++i )
		{
			if( mColliding[ i ].mDepth > depth )
			{
				deepest = i;
				depth = mColliding[ i ].mDepth;
			}
		}

		// try permutations swapping out the new pt for old ones.
		tVec4f options = tVec4f::cZeroVector;
		if( deepest != 0 ) options[ 0 ] = fCPSArea( pt.mBRel, 1, 2, 3, mColliding );
		if( deepest != 1 ) options[ 1 ] = fCPSArea( pt.mBRel, 0, 2, 3, mColliding );
		if( deepest != 2 ) options[ 2 ] = fCPSArea( pt.mBRel, 3, 0, 1, mColliding );
		if( deepest != 3 ) options[ 3 ] = fCPSArea( pt.mBRel, 2, 0, 1, mColliding );

		return options.fMaxAxisIndex( );
	}

	void tPersistentContactManifold::fClear( )
	{
		mColliding.fSetCount( 0 );
	}
	
	b32 tPersistentContactManifold::fForBody( const tPhysicsBody& b ) const 
	{ 
		return ( mA == &b || mB == &b ); 
	}
	
}}
