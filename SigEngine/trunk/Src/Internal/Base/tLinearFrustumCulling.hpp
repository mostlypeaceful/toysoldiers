#ifndef __tLinearFrustumCulling__
#define __tLinearFrustumCulling__

namespace Sig
{

	// Keep one of these as your key into the system.
	struct tLFCObject : public tRefCounter
	{
		declare_uncopyable( tLFCObject );
	public:
		u32 mIndex;

		tLFCObject( )
			: mIndex( ~0 )
		{ }

		b32 fInList( ) const
		{
			return mIndex != ~0;
		}
	};

	typedef tRefCounterPtr< tLFCObject > tLFCObjectPtr;
	
	class base_export tLinearFrustumCulling : public tRefCounter
	{
	public:		
		// tUserData is what you want back during an intersect, like a renderable entity.
		typedef void* tUserData;
		typedef u8 tFlags;

		static Math::tVec4f fToSphere( const Math::tObbf& bounds )
		{
			return Math::tVec4f( bounds.mCenter, bounds.mExtents.fLength( ) );
		}

		static Math::tVec4f fToSphere( const Math::tAabbf& bounds )
		{
			Math::tVec3f halfDiag = bounds.mMax - bounds.mMin;
			halfDiag *= 0.5f;
			return Math::tVec4f( bounds.mMin + halfDiag, halfDiag.fLength( ) );
		}

		// if size hasn't changed
		static void fMoveSphere( const Math::tAabbf& bounds, Math::tVec4f& outp )
		{
			Math::tVec3f halfDiag = bounds.mMax - bounds.mMin;
			halfDiag *= 0.5f;
			outp = Math::tVec4f( bounds.mMin + halfDiag, outp.w );
		}

	public:
		tLinearFrustumCulling( );
		~tLinearFrustumCulling( );

		void fClear( );

		void fInsert( const Math::tVec4f& sphere, const tLFCObjectPtr& object, tUserData userData, tFlags flags );
		void fMove	( const Math::tVec4f& sphere, const tLFCObjectPtr& object );
		void fUpdateFlags( const tLFCObjectPtr& object, tFlags flags );
		void fRemove( const tLFCObjectPtr& object );

		typedef u8 tFrustumResultMask;
		typedef tFixedGrowingArray< Math::tFrustumf, 8 > tFrustumArray;

		template< class tCallback >
		void fIntersect( const tFrustumArray& v, tCallback& cb )
		{
			fRefreshResults( v );

			tUserData*			data = mUserDatas.fBegin( );
			tFrustumResultMask*	results = mResults.fBegin( );
			tFlags*				flags = mFlags.fBegin( );

			while( results != mResults.fEnd( ) )
			{
				fPreFetch( data );
				fPreFetch( results );
				fPreFetch( flags );

				if( *results )
					cb( *data, *results, *flags );

				++data;
				++results;
				++flags;
			}
		}

	private:
		// Structure of arrays
		tGrowableArray< tLFCObjectPtr >	mObjects;
		tGrowableArray< tUserData >		mUserDatas;
		tGrowableArray< tFlags >		mFlags; //user defined use, supplemenatal data such as (shadow caster or not, LODed or not)
		tGrowableArray< Math::tVec4f >	mSpheres;
		tGrowableArray< tFrustumResultMask > mResults;

		// does actual culling.
		void fRefreshResults( const tFrustumArray& v );

		template< class t >
		void fPreFetch( t* from )
		{
#ifdef platform_xbox360
			// Fetches two cache lines full of t.
			__dcbt( (2 * 128) / sizeof( t ), from );
#endif
		}
	};
}

#endif//__tLinearFrustumCulling__
