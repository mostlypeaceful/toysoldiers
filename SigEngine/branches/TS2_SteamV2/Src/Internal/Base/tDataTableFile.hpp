#ifndef __tDataTableFile__
#define __tDataTableFile__
#include "tLocalizationFile.hpp"

namespace Sig
{
	class tDataTable;

	class base_export tDataTableCellArray : public Rtti::tSerializableBaseClass
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArray, 0x55B2830B );
	public:
		tDataTableCellArray( );
		tDataTableCellArray( tNoOpTag );
		virtual u32 fCellCount( ) const { return 0; }
		template<class t>
		t fValue( u32 icolumn, const tDataTable* dataTable ) const
		{
			t o = t( );
			fValuePtr( icolumn, Rtti::fGetClassId<t>( ), &o, dataTable );
			return o;
		}
	protected:
		virtual void fValuePtr( u32 icolumn, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const { }
	};

	class base_export tDataTable
	{
		declare_reflector( );
	public:
		tLoadInPlaceStringPtr*	mName;
		b32						mByColumn;
		tDynamicArray< tLoadInPlacePtrWrapper< tLoadInPlaceStringPtr > >	mColNames;
		tDynamicArray< tLoadInPlacePtrWrapper< tLoadInPlaceStringPtr > >	mRowNames;
		tDynamicArray< tLoadInPlacePtrWrapper< tDataTableCellArray > >		mCellArrays;

	public:
		tDataTable( );
		tDataTable( tNoOpTag );
		~tDataTable( );

		const tStringPtr& fName( ) const { return mName->fGetStringPtr( ); }

		inline u32 fColCount( ) const { return mColNames.fCount( ); }
		inline u32 fRowCount( ) const { return mRowNames.fCount( ); }

		u32 fColIndex( const tStringPtr& name ) const;
		u32 fRowIndex( const tStringPtr& name ) const;

		const tStringPtr& fColName( u32 i ) const { return mByColumn ? mColNames[ i ]->fGetStringPtr( ) : mRowNames[ i ]->fGetStringPtr( ); }
		const tStringPtr& fRowName( u32 i ) const { return mByColumn ? mRowNames[ i ]->fGetStringPtr( ) : mColNames[ i ]->fGetStringPtr( ); }

		template<class t>
		inline t fIndexByRowCol( u32 row, u32 col ) const
		{
			if( mByColumn )
				fSwap( row, col );
			if( row >= mCellArrays.fCount( ) )
			{
				log_assert( false, "Invalid " << ( mByColumn ? "column" : "row" ) << " index of " << row << " to table [" << fName( ) << "]" );
				return t( );
			}
			return mCellArrays[ row ]->fValue<t>( col, this );
		}
		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, u32 col ) const
		{
			return fIndexByRowCol<t>( fRowIndex( row ), col );
		}
		template<class t>
		inline t fIndexByRowCol( u32 row, const tStringPtr& col ) const
		{
			return fIndexByRowCol<t>( row, fColIndex( col ) );
		}
		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, const tStringPtr& col ) const
		{
			return fIndexByRowCol<t>( fRowIndex( row ), fColIndex( col ) );
		}

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Encapsulates the idea of a list of 2D data tables, 
	/// i.e., a single page from an excel book.
	class base_export tDataTableFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tDataTableFile, 0xA687305F);
	public:

		tDynamicArray< tLoadInPlacePtrWrapper<tDataTable> > mTables;

	public:
		static const u32 cVersion;
		static tFilePathPtr fMakeBinaryPath( const tFilePathPtr& excelFilePath );

	public:
		tDataTableFile( );
		tDataTableFile( tNoOpTag );
		virtual ~tDataTableFile( );
		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( );

		const tDataTable* fFindTable( const tStringPtr& tableName ) const;
		const tDataTable& fIndexTable( u32 index ) const { return *mTables[ index ]; }
		u32 fTableCount( ) const { return mTables.fCount( ); }
		u32 fTableIndex( const tStringPtr& tableName ) const;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};


	class base_export tDataTableCellArrayNumeric : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayNumeric, 0xF7FEA90A );
	private:
		tDynamicArray<f32> mValues;
	public:
		tDataTableCellArrayNumeric( );
		tDataTableCellArrayNumeric( const tDynamicArray<f32>& vals );
		tDataTableCellArrayNumeric( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 icolumn, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			if( icolumn >= mValues.fCount( ) )
			{
				log_assert( false, "Invalid " << ( dataTable->mByColumn ? "row" : "column" ) << " index of " << icolumn << " to table [" << dataTable->fName( ) << "]" );
				return;
			}
			else if( cid == Rtti::fGetClassId<f32>( ) )
				*( f32* )o = mValues[ icolumn ];
			else if( cid == Rtti::fGetClassId<u32>( ) )
				*( u32* )o = ( u32 )mValues[ icolumn ];
			else if( cid == Rtti::fGetClassId<s32>( ) )
				*( s32* )o = ( s32 )mValues[ icolumn ];
			else if( cid == Rtti::fGetClassId<bool>( ) )
				*( bool* )o = mValues[ icolumn ]!=0.f;
			else
				sigassert( "Invalid type requested in a numeric data table column." );
		}
	};

	class base_export tDataTableCellArrayStringPtr : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayStringPtr, 0xFCBE93E4 );
	private:
		tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> > mValues;
	public:
		tDataTableCellArrayStringPtr( );
		tDataTableCellArrayStringPtr( const tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceStringPtr> >& vals );
		tDataTableCellArrayStringPtr( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 icolumn, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			if( icolumn >= mValues.fCount( ) )
			{
				log_assert( false, "Invalid " << ( dataTable->mByColumn ? "row" : "column" ) << " index of " << icolumn << " to table [" << dataTable->fName( ) << "]" );
				return;
			}
			else if( cid == Rtti::fGetClassId<tStringPtr>( ) )
				*( tStringPtr* )o = mValues[ icolumn ]->fGetStringPtr( );
			else if( cid == Rtti::fGetClassId<tFilePathPtr>( ) )
				*( tFilePathPtr* )o = tFilePathPtr( mValues[ icolumn ]->fGetStringPtr( ).fCStr( ) );
			else
				sigassert( "Invalid type requested in a string data table column." );
		}
	};

	class base_export tDataTableCellArrayUnicodeString : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayUnicodeString, 0x26088CCC );
	private:
		tDynamicArray< tLocalizedString > mValues;
	public:
		tDataTableCellArrayUnicodeString( );
		tDataTableCellArrayUnicodeString( const tDynamicArray< tLocalizedString >& vals );
		tDataTableCellArrayUnicodeString( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 icolumn, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			if( icolumn >= mValues.fCount( ) )
			{
				log_assert( false, "Invalid " << ( dataTable->mByColumn ? "row" : "column" ) << " index of " << icolumn << " to table [" << dataTable->fName( ) << "]" );
				return;
			}
			else if( cid == Rtti::fGetClassId<tLocalizedString>( ) )
				*( tLocalizedString* )o = mValues[ icolumn ];
			else
				sigassert( "Invalid type requested in a unicode string data table column." );
		}
	};

	class base_export tDataTableCellArrayVector4 : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayVector4, 0xC0629D17 );
	private:
		tDynamicArray< Math::tVec4f > mValues;
	public:
		tDataTableCellArrayVector4( );
		tDataTableCellArrayVector4( const tDynamicArray<Math::tVec4f>& vals );
		tDataTableCellArrayVector4( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 icolumn, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			if( icolumn >= mValues.fCount( ) )
			{
				log_assert( false, "Invalid " << ( dataTable->mByColumn ? "row" : "column" ) << " index of " << icolumn << " to table [" << dataTable->fName( ) << "]" );
				return;
			}
			else if( cid == Rtti::fGetClassId<Math::tVec4f>( ) )
				*( Math::tVec4f* )o = mValues[ icolumn ];
			else
				sigassert( "Invalid type requested in a vector data table column." );
		}
	};


	///
	/// \brief String hash indexing wrapping a data table
	class tStringHashDataTable
	{
	public:
		void fSet( tResourcePtr& tables, u32 index );
		b32  fIsSet( ) const { return mTables.fGetRawPtr( ) != NULL; }

		u32  fRowIndex( const tStringPtr& row ) const;

		const tDataTable& fTable( ) const { return *mTable; }

		template<class t>
		inline t fIndexByRowCol( u32 row, u32 col ) const
		{
			return mTable->fIndexByRowCol<t>( row, col );
		}
		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, u32 col ) const
		{
			return fIndexByRowCol<t>( fRowIndex( row ), col );
		}

		typedef tHashTable< tStringPtr, u32 > tHash;
	private:
		tHash			mHash;
		tResourcePtr	mTables;
		const tDataTable *mTable;
	};

	///
	/// \brief Wraps a whole file with hashed tables.
	class base_export tStringHashDataTableFile
	{
	public:
		void fSet( const tResourcePtr& tables );
		b32  fIsSet( ) const { return mTables.fGetRawPtr( ) != NULL; }
		void fRefreshHash( );

		const tStringHashDataTable* fFindTable( const tStringPtr& tableName ) const;
		const tStringHashDataTable& fIndexTable( u32 index ) const { return mHashes[ index ]; }
		u32 fTableCount( ) const { return mHashes.fCount( ); }

	private:
		// raw storage/usage
		tDataTableFile& fTables( ) const { return *mTables->fCast<tDataTableFile>( ); }
		tResourcePtr mTables;

		// hashed dadta
		typedef tHashTable< tStringPtr, u32 > tHash;
		tHash mTableHash; //hashed lookup of table names
		tGrowableArray<tStringHashDataTable> mHashes;
	};
}

namespace Sig
{
	template<>
	class tResourceConvertPath<tDataTableFile>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddBB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubBB( path ); }
	};
}


#endif//__tDataTableFile__
