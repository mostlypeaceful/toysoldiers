#ifndef __tPotentialVisibilitySet__
#define __tPotentialVisibilitySet__

namespace Sig 
{ 
	namespace Math { class tConvexHull; }

	class tShapeEntity;

namespace Gfx
{
	class tCamera;
	class tVisibilitySetRef;

	/*

		A container intended to be stored at the scene graph level. 
		Large granularity culling is achieved by these "visibility sets."

		Sets can be enabled and disable to reduce strain on render cpu.
	*/
	class base_export tPotentialVisibilitySet : public tRefCounter
	{
	public:

		void fRegister( tEntity& ent, const tVisibilitySetRef& set );
		void fUnRegister( tEntity& ent, const tVisibilitySetRef& set );

		void fSetVisibility( const tStringPtr& name, b32 visible );

		void fClear( );

		// todo, make this work for multiple cameras
		void fUpdateForCamera( const Gfx::tCamera& camera );

		// These are the volumes that trigger the visiblity changes.
		//  Inverted means it will turn it off when you're in the volume.
		//  This function will NOT claim ownership of the hull and will only be referencing the original memory.
		void fRegisterConvexHull( tShapeEntity* shape, const tStringPtr& name, b32 inverted );


		struct tHullItem
		{
			b32 mInvered;
			tRefCounterPtr< tShapeEntity > mShape;

			tHullItem( tShapeEntity* shape = NULL, b32 inverted = false );
			~tHullItem( );

			b32 operator == ( const tShapeEntity* shape ) const { return mShape == shape; }
		};

		class tLayer : public tRefCounter
		{
		public:
			tLayer( const tStringPtr& name = tStringPtr::cNullPtr ) 
				: mVisible( true )
				, mName( name )
			{ }

			b32 mVisible;
			tStringPtr mName;
			tGrowableArray< tEntityPtr > mEntites;
			tGrowableArray< tHullItem > mHulls;
		};
	private:
		typedef tRefCounterPtr< tLayer > tLayerPtr;

		tGrowableArray< tLayerPtr > mLayers;

		tLayer* fLayer( const tStringPtr& name, b32 createIfNotFound );


		void fSetVisibility( tLayer& layer, b32 visible );
		static void fApplyVisibility( tEntity& root, b32 visible );
	};

	/*
		tVisibilitySetRef - Behaves like a render batch. There is only one instance for any set combination to reduce memory.
	*/

	class tVisibilitySetRef : public tRefCounter
	{
	public:
		tGrowableArray< tStringPtr > mSet;

		b32 operator == ( const tGrowableArray<tStringPtr>& other ) const
		{
			if( mSet.fCount( ) != other.fCount( ) )
				return false;

			for( u32 i = 0; i < mSet.fCount( ); ++i )
				if( !other.fFind( mSet[ i ] ) )
					return false;

			return true;
		}
	};

	typedef tRefCounterPtr< tVisibilitySetRef > tVisibilitySetRefPtr;

	class tVisibilitySetRefManager
	{
		declare_singleton( tVisibilitySetRefManager );
	public:
		tVisibilitySetRef* fMakeRef( const tGrowableArray<tStringPtr>& set )
		{
			for( u32 i = 0; i < mSets.fCount( ); ++i )
			{
				if( *mSets[ i ] == set )
					return mSets[ i ].fGetRawPtr( );
			}

			tVisibilitySetRef* ref = NEW tVisibilitySetRef( );
			ref->mSet = set;
			mSets.fPushBack( tVisibilitySetRefPtr( ref ) );
			return ref;
		}

		tVisibilitySetRef* fBlankRef( )
		{
			tGrowableArray<tStringPtr> blank;
			return fMakeRef( blank );
		}

	private:
		tGrowableArray< tVisibilitySetRefPtr > mSets;
	};

}}


#endif//__tPotentialVisibilitySet__
