#ifndef __tEditorSelectionList__
#define __tEditorSelectionList__
#include "tDelegate.hpp"
#include "tEntity.hpp"

namespace Sig
{
	namespace Gfx { struct tDisplayStats; }

	///
	/// \brief Represents a list of selected objects. There is a single "current"
	/// selection list that lives on the tEditorAppWindow instance, but you can
	/// save off selection lists by storing your own instances.
	class tools_export tEditorSelectionList
	{
	public:
		typedef tGrowableArray< tEntityPtr > tEntityArray;
		typedef tEvent<void ( tEditorSelectionList& )> tOnSelectionChanged;
	private:
		tEntityArray			mObjects;
		tOnSelectionChanged		mOnSelChanged;
	public:
		tEditorSelectionList( );
		void fAdd( const tEntityPtr& entity, b32 select = true );
		void fRemove( const tEntityPtr& entity, b32 deselect = true );
		void fClear( b32 deselect = true );
		void fReset( const tEditorSelectionList& newList, b32 modifySelection = true );
		void fDuplicateAndReplace( );

		inline u32 fCount( ) const { return mObjects.fCount( ); }
		const tEntityPtr& operator[]( u32 i ) const { return mObjects[ i ]; }
		tEntityPtr* fBegin( ) { return mObjects.fBegin( ); }
		tEntityPtr* fEnd( ) { return mObjects.fEnd( ); }

		inline b32 fContains( const tEntityPtr& entity ) const { return mObjects.fFind( entity ) != 0; }
		Math::tAabbf fComputeBounding( ) const;
		Math::tVec3f fComputeAveragePosition( ) const;
		void fComputeSelectedDisplayStats( Gfx::tDisplayStats& displayStatsOut ) const;
		void fSnapToGround( b32 onlySnapIfSpecified ) const;

		void fReselect( ) { mOnSelChanged.fFire( *this ); }

		inline tOnSelectionChanged&			fGetSelChangedEvent( )			{ return mOnSelChanged; }
		inline const tOnSelectionChanged&	fGetSelChangedEvent( ) const	{ return mOnSelChanged; }

		template<class t>
		void fCullByType( tGrowableArray< t* >& output ) const
		{
			for( u32 i = 0; i < fCount( ); ++i )
			{
				t* c = (*this)[ i ]->fDynamicCast< t >( );
				if( c )
					output.fPushBack( c );
			}
		}

	private:
		void fSelectAll( );
		void fDeSelectAll( );
	};

}

#endif//__tEditorSelectionList__
