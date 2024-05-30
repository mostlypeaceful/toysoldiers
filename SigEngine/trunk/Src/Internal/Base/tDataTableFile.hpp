#ifndef __tDataTableFile__
#define __tDataTableFile__
#include "tLocalizationFile.hpp"
#include "tDynamicBitArray.hpp"

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
		t fValue( u32 irow, const tDataTable* dataTable ) const
		{
			t o = t( );
			fValuePtr( irow, Rtti::fGetClassId<t>( ), &o, dataTable );
			return o;
		}
	protected:
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const { }
	};

	class base_export tDataTable
	{
		declare_reflector( );
	public:
		tLoadInPlaceStringPtr*	mName;
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

		u32 fColIndex( const tStringPtr& name ) const; ///< Get a column index from a column name, or ~0 if no such column exists.
		u32 fRowIndex( const tStringPtr& name ) const; ///< Get a row index from a row name, or ~0 if no such row exists.

		const tStringPtr& fColName( u32 i ) const { return mColNames[ i ]->fGetStringPtr( ); }
		const tStringPtr& fRowName( u32 i ) const { return mRowNames[ i ]->fGetStringPtr( ); }

		template<class t>
		inline t fIndexByRowCol( u32 row, u32 col ) const
		{
			if( col >= mCellArrays.fCount( ) )
			{
				log_assert( false, "Invalid column index of " << col << " to table [" << fName( ) << "]" );
				return t( );
			}
			return mCellArrays[ col ]->fValue<t>( row, this );
		}
		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, u32 col ) const
		{
			const u32 rowIndex = fRowIndex( row );
			log_assert( rowIndex < fRowCount( ), "Row [" << row << "] not found in table: " << fName( ) );

			return fIndexByRowCol<t>( rowIndex, col );
		}
		template<class t>
		inline t fIndexByRowCol( u32 row, const tStringPtr& col ) const
		{
			const u32 colIndex = fColIndex( col );
			log_assert( colIndex < fColCount( ), "Column [" << col << "] not found in table: " << fName( ) );

			return fIndexByRowCol<t>( row, colIndex );
		}
		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, const tStringPtr& col ) const
		{
			const u32 colIndex = fColIndex( col );
			log_assert( colIndex < fColCount( ), "Column [" << col << "] not found in table: " << fName( ) );

			return fIndexByRowCol<t>( row, colIndex );
		}

		template<class t>
		t fFindByRowCol( const tStringPtr& row, const tStringPtr& col ) const
		{
			const u32 colIndex = fColIndex( col );
			log_assert( colIndex < fColCount( ), "Column [" << col << "] not found in table: " << fName( ) );

			const u32 rowIndex = fRowIndex( row );
			if( rowIndex < fRowCount( ) )
				return mCellArrays[ colIndex ]->fValue<t>( rowIndex, this );

			return t( );
		}

	public:
		// Validation stuff
		struct ColumnTypes
		{
			enum tTypes
			{
				cString,
				cFloat,
				cDouble,
				cFilepath,
				cInt,
				cVector,
				cBool,
				cDateTime,
				cTypesCount
			};

			static const std::string mType[ cTypesCount ];
			static const std::string mDirective[ cTypesCount ];
		};

		// In case you can handle multiple types and need to figure out what to use.
		b32 fIsColumnType( u32 index, ColumnTypes::tTypes type ) const;

		// Call these for each column, with the type you expected.
		void fValidateColumn( u32 index, ColumnTypes::tTypes type ) const;

		// Then call this with the number of columns you expected.
		void fValidateCount( u32 count ) const;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Encapsulates the idea of a list of 2D data tables, 
	/// i.e., a single page from an excel book.
	class base_export tDataTableFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tDataTableFile, 0xA687305F);

	public:

		tDynamicArray< tLoadInPlacePtrWrapper<tDataTable> > mTables;

	public:
		static tFilePathPtr fMakeBinaryPath( const tFilePathPtr& excelFilePath );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddBB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathSubBB( path ); }

		typedef tDataTable tTableType;

	public:
		tDataTableFile( );
		tDataTableFile( tNoOpTag );
		virtual ~tDataTableFile( );
		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		const tDataTable* fFindTable( const tStringPtr& tableName ) const;
		const tDataTable& fGetTable( const tStringPtr & tableName ) const;
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
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );

			if( cid == Rtti::fGetClassId<f32>( ) )
				*( f32* )o = mValues[ irow ];
			else if( cid == Rtti::fGetClassId<u32>( ) )
				*( u32* )o = ( u32 )mValues[ irow ];
			else if( cid == Rtti::fGetClassId<s32>( ) )
				*( s32* )o = ( s32 )mValues[ irow ];
			else
				log_assert( false, "Invalid type requested in a numeric column of data table [" << dataTable->fName( ) << "]" );
		}
	};

	class base_export tDataTableCellArrayDoubleNumeric : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayDoubleNumeric, 0x92E38071 );
	private:
		tDynamicArray<f64> mValues;
	public:
		tDataTableCellArrayDoubleNumeric( );
		tDataTableCellArrayDoubleNumeric( const tDynamicArray<f64>& vals );
		tDataTableCellArrayDoubleNumeric( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );

			if( cid == Rtti::fGetClassId<f64>( ) )
				*( f64* )o = mValues[ irow ];
			else if( cid == Rtti::fGetClassId<u64>( ) )
				*( u64* )o = ( u64 )mValues[ irow ];
			else if( cid == Rtti::fGetClassId<s64>( ) )
				*( s64* )o = ( s64 )mValues[ irow ];
			else
				log_assert( false, "Invalid type requested in a numeric column of data table [" << dataTable->fName( ) << "]" );
		}
	};

	class base_export tDataTableCellArrayBoolean : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayBoolean, 0x6172C9C5 );
	private:
		u32 mValueCount;
		tDynamicBitArray<u32> mValues;
	public:
		tDataTableCellArrayBoolean( );
		tDataTableCellArrayBoolean( const tDynamicArray<b32>& vals );
		tDataTableCellArrayBoolean( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValueCount; }
	protected:
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValueCount, "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );

			if( cid == Rtti::fGetClassId<b32>( ) )
				*( b32* )o = mValues.fGetBit( irow );
			else if( cid == Rtti::fGetClassId<bool>( ) )
				*( bool* )o = ( mValues.fGetBit( irow ) ? true : false );
			else
				log_assert( false, "Invalid type requested in a boolean column of data table [" << dataTable->fName( ) << "]" );
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
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );
			log_sigcheckfail( cid == Rtti::fGetClassId<tStringPtr>( ), "Invalid type requested in a string column of data table [" << dataTable->fName( ) << "]", return );
			*( tStringPtr* )o = ( mValues[ irow ] ? mValues[ irow ]->fGetStringPtr( ) : tStringPtr::cNullPtr );
		}
	};

	class base_export tDataTableCellArrayFilePathPtr : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayFilePathPtr, 0x286FA4F9 );
	private:
		tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceFilePathPtr> > mValues;
	public:
		tDataTableCellArrayFilePathPtr( );
		tDataTableCellArrayFilePathPtr( const tDynamicArray< tLoadInPlacePtrWrapper<tLoadInPlaceFilePathPtr> >& vals );
		tDataTableCellArrayFilePathPtr( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );
			log_sigcheckfail( cid == Rtti::fGetClassId<tFilePathPtr>( ), "Invalid type requested in a file column of data table [" << dataTable->fName( ) << "]", return );
			*( tFilePathPtr* )o = ( mValues[ irow ] ? mValues[ irow ]->fGetFilePathPtr( ) : tFilePathPtr::cNullPtr );
		}
	};

	class base_export tDataTableCellArrayDateTime : public tDataTableCellArray
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDataTableCellArrayDateTime, 0xC53B3BD6 );
	private:
		tDynamicArray< u64 > mValues;
	public:
		tDataTableCellArrayDateTime( );
		tDataTableCellArrayDateTime( const tDynamicArray< u64 >& vals );
		tDataTableCellArrayDateTime( tNoOpTag );
		virtual u32 fCellCount( ) const { return mValues.fCount( ); }
	protected:
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );
			log_sigcheckfail( cid == Rtti::fGetClassId<Time::tDateTime>( ), "Invalid type requested in a file column of data table [" << dataTable->fName( ) << "]", return );
			*( Time::tDateTime* )o = Time::tDateTime::fFromUnixTimeUTC( mValues[ irow ] );
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
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );
			log_sigcheckfail( cid == Rtti::fGetClassId<tLocalizedString>( ), "Invalid type requested in a unicode string column of data table [" << dataTable->fName( ) << "]", return );
			*( tLocalizedString* )o = mValues[ irow ];
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
		virtual void fValuePtr( u32 irow, Rtti::tClassId cid, void* o, const tDataTable* dataTable ) const
		{
			log_sigcheckfail( irow < mValues.fCount( ), "Invalid row index of " << irow << " to table [" << dataTable->fName( ) << "]", return );
			log_sigcheckfail( cid == Rtti::fGetClassId<Math::tVec4f>( ), "Invalid type requested in a vector column of data table [" << dataTable->fName( ) << "]", return );
			*( Math::tVec4f* )o = mValues[ irow ];
		}
	};

	///
	/// \brief String hash indexing wrapping a data table
	class base_export tStringHashDataTable
	{
	public:
		void fSet( tResourcePtr& tables, u32 index );
		b32  fIsSet( ) const { return mTables.fGetRawPtr( ) != NULL; }

		u32  fRowIndex( const tStringPtr& row ) const;
		const tStringPtr& fRowName( u32 index ) const { return mTable->fRowName( index ); }

		const tDataTable& fTable( ) const { return *mTable; }

		template<class t>
		inline t fIndexByRowCol( u32 row, u32 col ) const
		{
			return mTable->fIndexByRowCol<t>( row, col );
		}

		template<class t>
		inline t fIndexByRowCol( const tStringPtr& row, u32 col ) const
		{
			const u32 rowIndex = fRowIndex( row );
			log_assert( rowIndex < fTable( ).fRowCount( ), "Row [" << row << "] not found in table: " << fTable( ).fName( ) );

			return fIndexByRowCol<t>( rowIndex, col );
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
		tStringHashDataTableFile( ) { }
		tStringHashDataTableFile( tResourcePtr& tables ) { fSet( tables ); }

		void fSet( const tResourcePtr& tables );
		b32  fIsSet( ) const { return mTables.fGetRawPtr( ) != NULL; }
		void fRefreshHash( );

		const tStringHashDataTable* fFindTable( const tStringPtr& tableName ) const;
		const tStringHashDataTable& fIndexTable( u32 index ) const { return mHashes[ index ]; }
		u32 fTableCount( ) const { return mHashes.fCount( ); }
		const tFilePathPtr& fGetPath( ) const { return mTables->fGetPath( ); }

	private:
		// raw storage/usage
		tDataTableFile& fTables( ) const { tDataTableFile* dtf = mTables->fCast<tDataTableFile>( ); sigassert( dtf ); return *dtf; }
		tResourcePtr mTables;

		// hashed dadta
		typedef tHashTable< tStringPtr, u32 > tHash;
		tHash mTableHash; //hashed lookup of table names
		tGrowableArray<tStringHashDataTable> mHashes;
	};


} // ::Sig

#endif//__tDataTableFile__
