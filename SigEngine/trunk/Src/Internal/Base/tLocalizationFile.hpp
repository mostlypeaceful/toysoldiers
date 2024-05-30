#ifndef __tLocalizationFile__
#define __tLocalizationFile__

namespace Sig
{
	///
	/// \brief Our custom load-in-place serializable localized (unicode) string type.
	class base_export tLocalizedString : public tDynamicArray<u16>
	{
		declare_reflector( );
		sig_make_stringstreamable( tLocalizedString, fCStr( ) );
	public:
		static void fExportScriptInterface( tScriptVm& vm );

		static tLocalizedString fConstructMoneyString( const char* moneyString );
		static tLocalizedString fConstructTimeString( f32 time, b32 showPartialSeconds );
		static tLocalizedString fConstructTimeStringFromSeconds( s64 totalSeconds );
		static tLocalizedString fLocalizeNumber( f32 val );
		static tLocalizedString fLocalizeNumber( s32 val );
		static tLocalizedString fLocalizeNumber( s64 val );
		static tLocalizedString fLocalizeNumberScript( Sqrat::Object val );
		static const tLocalizedString& fEmptyString( );
		static const tLocalizedString& fNullString( );
		static tLocalizedString fFromCString( const char* s );

	public:
		tLocalizedString( ) { }
		~tLocalizedString( );

		explicit tLocalizedString( const std::string& s ) { fFromCStr( s.c_str( ), ( u32 )s.length( ) ); }
		explicit tLocalizedString( const wchar_t* s ) { fFromCStr( s ); }
		explicit tLocalizedString( const std::wstring& s ) { fFromCStr( s.c_str( ), ( u32 )s.length( ) ); }

		void fFromCStr( const char* s, u32 len = ~0 );
		void fFromCStr( const u16* s, u32 len = ~0 );
		void fFromCStr( const wchar_t* s, u32 len = ~0 );

		const wchar_t* fCStr( ) const { return fCount( ) ? ( const wchar_t* )fBegin( ) : L""; }
		std::string fToCString( ) const { return fLength( ) ? StringUtil::fWStringToString( fCStr( ) ) : std::string( ); }
		std::wstring fToWCString( ) const { return fLength( ) ? std::wstring( fCStr( ) ) : std::wstring( ); }
		u32 fLength( ) const { return fCount( ) ? fCount( ) - 1 : 0; }

		tLocalizedString& fJoinWithLocString( const tLocalizedString& locStr );
		tLocalizedString& fJoinWithCString( const wchar_t * str, u32 len = ~0 );
		tLocalizedString& fJoinWithCString( const char* str, u32 len = ~0 );
		

		tLocalizedString fClone( ) const;
		
		tLocalizedString fSlice( u32 start, u32 end ) const;

		b32 fIsWhiteSpace( u32 index ) const;

		tLocalizedString& fReplace( const char* macroId, const char* locKey ); 
		tLocalizedString& fReplace( const char* macroId, const tLocalizedString& locStr ); 
		tLocalizedString& fReplaceValue( const char* macroId, s32 val );
		tLocalizedString& fReplaceValue( const char* macroId, u32 val );
		tLocalizedString& fReplaceValue( const char* macroId, f32 val );
		tLocalizedString& fReplaceCString( const char* macroId, const char* str );
		tLocalizedString& fReplaceScript( const char* macroId, Sqrat::Object loc );

		b32 fCompare( const tLocalizedString& rhs );

		void fStripNewlines( );

	private:

		tLocalizedString& fJoinWithCStringForScript( const char* str ) { return fJoinWithCString( str ); }
	};

	///
	/// \brief Provides localization support, including unicode string table storage by tStringPtr ID,
	/// as well as locale-specific resource paths (also accessible by tStringPtr ID).
	class base_export tLocalizationFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tLocalizationFile, 0xD5CB5860);
	public:

		struct base_export tStringEntry
		{
			declare_reflector( );

			tLoadInPlaceStringPtr*		mId;
			tLocalizedString			mText;

			tStringEntry( ) : mId( 0 ) { }
		};

		typedef tDynamicArray<tStringEntry>														tStringArray;
		typedef tHashTable<tHashTablePtrInt, const tStringEntry*, tHashTableNoResizePolicy>		tStringMap;
		typedef tLoadInPlaceRuntimeObject<tStringMap>											tStringMapStorage;

		struct base_export tPathEntry
		{
			declare_reflector( );

			tLoadInPlaceStringPtr*		mId;
			tLoadInPlaceResourceId*		mPath;

			tPathEntry( ) : mId( 0 ), mPath( 0 ) { }
		};

		typedef tDynamicArray<tPathEntry>														tPathArray;
		typedef tHashTable<tHashTablePtrInt, const tPathEntry*, tHashTableNoResizePolicy>		tPathMap;
		typedef tLoadInPlaceRuntimeObject<tPathMap>												tPathMapStorage;

	public:
		static const char* fGetFileExtension( );
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }

	public:
		tStringArray			mRawStrings;
		tStringMapStorage		mStringMap;
		tPathArray				mRawPaths;
		tPathMapStorage			mPathMap;

	public:
		tLocalizationFile( );
		tLocalizationFile( tNoOpTag );
		~tLocalizationFile( );

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		s32 fStringIndexFromId( const tStringPtr& id ) const; // returns < 0 if id is not present, performs a hash table lookup
		const tLocalizedString& fStringFromId( const tStringPtr& id ) const; // hash table lookup, essentially equivalent to fStringFromIndex( fIndexFromId( id ) ); if not found returns default empty string
		const tLocalizedString& fStringFromIndex( u32 index ) const; // direct array access, fastest option (assumes index is valid, otherwise asserts)

		u32 fPathCount( ) const { return mRawPaths.fCount( ); }
		s32 fPathIndexFromId( const tStringPtr& id ) const; // returns < 0 if id is not present, performs a hash table lookup
		tFilePathPtr fPathFromId( const tStringPtr& id ) const; // hash table lookup, essentially equivalent to fStringFromIndex( fIndexFromId( id ) )
		tFilePathPtr fPathFromIndex( u32 index ) const; // direct array access, fastest option
		tStringPtr	 fPathIdFromIndex( u32 index) const;

		b32			 fJoinLocFile( const tLocalizationFile& ); //adds contents of the argument loc file to the hash map of this loc file.
	};

} // ::Sig


#endif//__tLocalizationFile__

