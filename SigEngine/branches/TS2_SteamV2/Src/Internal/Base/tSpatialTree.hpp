#ifndef __tSpatialTree__
#define __tSpatialTree__

namespace Sig
{

	class tSpatialTree
	{
		declare_null_reflector( );
	public:

		template<class tDerivedTree>
		struct tCellKey
		{
			tDerivedTree*	mCell;
			u32				mIndex;

			inline tCellKey( ) : mCell( 0 ), mIndex( ~0 ) { }
			inline tCellKey( tDerivedTree* cell, u32 index ) : mCell( cell ), mIndex( index ) { }
			inline b32 fNull( ) const { return mCell == 0 || mIndex == ~0; }
		};

		///
		/// \brief Represents the minimal data and functionality required to be insertable into a dynamic octree.
		/// \note This class represents a nice compact and "evenly" sized object; take care if you need to
		/// add data members (read: you shouldn't add data or give this class a virtual table or anything like that).
		template<class tDerivedTree>
		class tSpatialObject : public tUncopyable
		{
		public:
			Math::tAabbf					mWorldSpaceBox; // 2 * sizeof( vec3f ) = 2 * ( 3 * 4 ) = 24 bytes
			mutable tCellKey<tDerivedTree>	mCellKey; // 2 * sizeof( int ) = 2 * 4 = 8 bytes // DON'T TOUCH THIS!

		public:
			inline tSpatialObject( )
			{
			}

			inline ~tSpatialObject( )
			{
				if( !mCellKey.fNull( ) )
					fRemove( );
			}

			inline b32 fQuickRejectByFlags( ) const
			{
				//return ( fFlags( ) & cFlagDisabled );
				return false;
			}

			inline b32 fQuickRejectByBox( const Math::tRayf& ray ) const
			{
				return !mWorldSpaceBox.fIntersectsOrContains( ray );
			}

			void fRemove( )
			{
				sigassert( !mCellKey.fNull( ) );
				mCellKey.mCell->fRemove( mCellKey.mIndex );
				mCellKey = tCellKey<tDerivedTree>( 0, ~0 );
			}

			tDerivedTree* fMove( u32 maxDepth )
			{
				sigassert( !mCellKey.fNull( ) );

				// we remove ourself from the current cell, but then immediately try to re-insert;
				// we inform the caller of success or failure so that it can try to insert at the root if necessary
				tDerivedTree* currentCell = mCellKey.mCell;
				fRemove( );
				const b32 success = currentCell->fInsert( this, currentCell->fDepth( ), maxDepth );
				return success ? 0 : currentCell;
			}
		};

	};
}

#endif//__tSpatialTree__
