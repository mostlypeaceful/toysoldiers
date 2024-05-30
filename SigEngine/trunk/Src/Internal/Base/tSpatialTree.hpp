#ifndef __tSpatialTree__
#define __tSpatialTree__

namespace Sig
{

	class tSpatialEntity;

	class tSpatialTree
	{
		declare_null_reflector( );
	public:

		// this used to be templated, but for no good reason.
		//  using a little bit riskier code for memory allocation control
		struct tCellKey
		{
			void*	mCell;
			u32				mIndex;

			inline tCellKey( ) : mCell( 0 ), mIndex( ~0 ) { }
			inline tCellKey( void* cell, u32 index ) : mCell( cell ), mIndex( index ) { }
			inline b32 fNull( ) const { return mCell == 0 || mIndex == ~0; }
		};

		///
		/// \brief Represents the minimal data and functionality required to be insertable into a dynamic octree.
		/// \note This class represents a nice compact and "evenly" sized object; take care if you need to
		/// add data members (read: you shouldn't add data or give this class a virtual table or anything like that).
		class tSpatialObject : public tUncopyable, public tRefCounter
		{
			define_class_pool_new_delete( tSpatialObject, 512 ); //only possible when not invasive
		private:
			tSpatialEntity*				mOwner; //currently is invasive

		public:
			Math::tAabbf					mWorldSpaceBox; // 2 * sizeof( vec3f ) = 2 * ( 3 * 4 )	= 24 bytes
			mutable tCellKey				mCellKey; // 2 * sizeof( int ) = 2 * 4					= 8 bytes
			
		public:
			inline tSpatialObject( tSpatialEntity* owner = NULL )
				: mOwner( owner )
			{
			}

			inline ~tSpatialObject( )
			{
				sigassert( mCellKey.fNull( ) );
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

			template< class tDerivedType >
			void fRemove( )
			{
				sigassert( !mCellKey.fNull( ) );
				static_cast<tDerivedType*>( mCellKey.mCell )->fRemove( mCellKey.mIndex );
				mCellKey = tCellKey( 0, ~0 );
			}

			template< class tDerivedType >
			tDerivedType* fMove( u32 maxDepth )
			{
				sigassert( !mCellKey.fNull( ) );

				// we remove ourself from the current cell, but then immediately try to re-insert;
				// we inform the caller of success or failure so that it can try to insert at the root if necessary
				tDerivedType* currentCell = static_cast<tDerivedType*>( mCellKey.mCell );
				fRemove<tDerivedType>( );
				const b32 success = currentCell->fInsert( this, currentCell->fDepth( ), maxDepth );
				return success ? 0 : currentCell;
			}

			tSpatialEntity* fOwner( )
			{
				//if this becomes invasive, return "this"
				return mOwner;
			}
		};

	};
}

#endif//__tSpatialTree__
