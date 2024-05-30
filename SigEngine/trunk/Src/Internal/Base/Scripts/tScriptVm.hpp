#ifndef __tScriptVm__
#define __tScriptVm__
#include "tResource.hpp"
#include "SqratInclude.hpp"

namespace Sig
{
	class tScriptVm;
	class tScriptFile;
	class tResource;
	class tResourcePtr;
}

namespace Sig
{
	///
	/// \brief Wraps the squirrel virtual machine stack object. Allows for registering
	/// native functions and compiling scripts from string or bytecode.
	class base_export tScriptVm : public tUncopyable
	{
		declare_singleton_define_own_ctor_dtor( tScriptVm );

	public:
		struct tScriptNamespace
		{
			const char*		mName;
			Sqrat::Table*	mTable;
			tScriptNamespace( ) : mName( 0 ), mTable( 0 ) { }
			tScriptNamespace( const char* name, Sqrat::Table* table ) : mName( name ), mTable( table ) { }
			inline b32 operator==( const char* name ) const { return strcmp( mName, name ) == 0; }
		};
	private:
		HSQUIRRELVM												mSq;
		Sqrat::Table*											mEmptyTable;
		Sqrat::RootTable*										mRootTable;
		Sqrat::ConstTable*										mConstTable;
		tGrowableArray<tScriptNamespace>						mNamespaces;

		class base_export tTimeStamps :
			public tHashTable< tFilePathPtr, u64, tHashTableExpandOnlyResizePolicy >
		{
		} mTimeStamps;

	public:
		tScriptVm( );
		~tScriptVm( );
		inline HSQUIRRELVM fSq( ) const { return mSq; }
		inline Sqrat::Table& fEmptyTable( ) { return *mEmptyTable; }
		inline Sqrat::RootTable& fRootTable( ) { return *mRootTable; }
		inline Sqrat::ConstTable& fConstTable( ) { return *mConstTable; }
		Sqrat::Table& fNamespace( const char* name );
		void fReset( );
		u32 fQueryStackTop( ) const;
		void fLogStackTop( const char* context = 0, u32 minValueAtWhichToLog = 0 ) const;
		b32 fGarbageCollect( );
		b32 fCompileString( const char* str, const char* srcFileName );
		b32 fCompileStringAndRun( const char* str, const char* srcFileName );
		b32 fGenerateByteCode( tDynamicBuffer& byteCode, b32 endianSwap );
		b32 fRegisterScriptFile( tScriptFile& scriptFile, const tResource& res );
	private:
		void fNew( );
		void fClear( );
		b32 fRunCurrent( );
	public:
		void fExportGameFlags(
			const tStringPtr      gameFlagNames[],
			const u32             gameFlagValues[],
			const tStringPtr      gameEnumTypeNames[],
			const u32             gameEnumTypeKeys[],
			const tStringPtr*const gameEnumValueNames[],
			const u32*const       gameEnumValues[] );
		void fExportCommonScriptInterfaces( );
		static void fDumpCallstack( );
	};

}


#endif//__tScriptVm__
