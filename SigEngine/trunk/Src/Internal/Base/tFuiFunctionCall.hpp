#ifndef __tFuiFunctionCall__
#define __tFuiFunctionCall__

#include "tLocalizationFile.hpp"
#include "iggy.h"

namespace Sig
{
	// New prefered function call style:
	/* Use it like this:

		tFuiFuncCall<void ( u32, f32, char* )> call( "fTest", myFui.fHandle( ) );
		call.fExecute( 2, 1.5f, "testStr" );

		You can construct these once and keep them around, calling fExecute when ever necesary for full optimization.
		
		They are safe to copy.
		And you can defer initialization like this:

		tFuiFuncCall<void ( u32, f32, char* )> call;
		call.fSet( "fTest", myFui.fHandle( ) );
		call.fExecute( 2, 1.5f, "testStr" );
	*/

	template< u32 cParamCount, u32 cArraySize > 
	struct tFuncCallData
	{
		void fSet( const char* name, Iggy* fui )
		{
			mHandle = fui;
			mFunctionName = IggyPlayerCreateFastNameUTF8( mHandle, name, strlen( name ) );
		}

        b32 fValid( ) const { return mHandle != NULL; }
		Iggy* fHandle( ) const { return mHandle; }

	protected:
		tFuncCallData( )
			: mHandle( NULL )
			, mFunctionName( 0 )
		{
			sigassert( cParamCount <= cArraySize );
		}

		tFixedArray< IggyDataValue, cArraySize > mArgs;
		IggyName mFunctionName;
		Iggy* mHandle;

		template< typename t >
		void fSetArgType( u32 index )
		{
			sigassert( "!This should not be getting called! Double check overloads below" );
		}
		template< >
		void fSetArgType<f32>( u32 index )
		{
			mArgs[ index ].type = IGGY_DATATYPE_number;
		}
		template< >
		void fSetArgType<u32>( u32 index )
		{
			mArgs[ index ].type = IGGY_DATATYPE_number;
		}
		template< >
		void fSetArgType<s32>( u32 index )
		{
			mArgs[ index ].type = IGGY_DATATYPE_number;
		}
		template< >
		void fSetArgType<char*>( u32 index )
		{
			mArgs[ index ].type = IGGY_DATATYPE_string_UTF8;
		}
        template< >
        void fSetArgType<const char*>( u32 index )
        {
            mArgs[ index ].type = IGGY_DATATYPE_string_UTF8;
        }
		template< >
		void fSetArgType<wchar_t*>( u32 index )
		{
			mArgs[ index ].type = IGGY_DATATYPE_string_UTF16;
		}
        template< >
        void fSetArgType<const wchar_t*>( u32 index )
        {
            mArgs[ index ].type = IGGY_DATATYPE_string_UTF16;
        }

		void fSetArg( u32 index, f32 val )
		{
			IggyDataValue* arg = &mArgs[ index ];
			sigassert( arg->type == IGGY_DATATYPE_number );
			arg->number = val;
		}
		void fSetArg( u32 index, u32 val )
		{
			IggyDataValue* arg = &mArgs[ index ];
			sigassert( arg->type == IGGY_DATATYPE_number );
			arg->number = val;
		}
		void fSetArg( u32 index, s32 val )
		{
			IggyDataValue* arg = &mArgs[ index ];
			sigassert( arg->type == IGGY_DATATYPE_number );
			arg->number = val;
		}
		void fSetArg( u32 index, const char* val )
		{
			IggyDataValue* arg = &mArgs[ index ];
			sigassert( arg->type == IGGY_DATATYPE_string_UTF8 );
			arg->string8.string = const_cast< char* >( val );
			arg->string8.length = strlen( val );
		}
		void fSetArg( u32 index, const wchar_t* val )
		{
			IggyDataValue* arg = &mArgs[ index ];
			sigassert( arg->type == IGGY_DATATYPE_string_UTF16 );
			arg->string16.string = const_cast< wchar_t* >( val );
			arg->string16.length = wcslen( val );
		}

		b32 fExecuteInternal( )
		{
			const IggyResult result = IggyPlayerCallFunctionRS( mHandle, NULL, mFunctionName, cParamCount, mArgs.fBegin( ) );
			if( result != IGGY_RESULT_SUCCESS && Log::fFlagEnabled( Log::cFlagFui ) )
				log_warning( "tFui::fCallFunction [" << mFunctionName << "] failed" );
			return result == IGGY_RESULT_SUCCESS;
		}
	};

	template< typename tSignature >
	struct tFuiFuncCall : public tFuncCallData< 1, 1 >
	{
		/* dont let this one compile! */
	};

	template< >
	struct tFuiFuncCall< void ( ) > : public tFuncCallData< 0, 1 > // needs to be greater than zero
	{
		tFuiFuncCall( ) { }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fSet( name, fui );
		}

		b32 fExecute( ) 
		{ 
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1 >
	struct tFuiFuncCall< void ( tA1 ) > : public tFuncCallData< 1, 1 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
		}

		b32 fExecute( const tA1& a1 ) 
		{ 
			fSetArg( 0, a1 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2 >
	struct tFuiFuncCall< void ( tA1, tA2 ) > : public tFuncCallData< 2, 2 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2, typename tA3 >
	struct tFuiFuncCall< void ( tA1, tA2, tA3 ) > : public tFuncCallData< 3, 3 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
			fSetArgType<tA3>( 2 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2, const tA3& a3 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			fSetArg( 2, a3 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2, typename tA3, typename tA4 >
	struct tFuiFuncCall< void ( tA1, tA2, tA3, tA4 ) > : public tFuncCallData< 4, 4 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
			fSetArgType<tA3>( 2 );
			fSetArgType<tA4>( 3 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2, const tA3& a3, const tA4& a4 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			fSetArg( 2, a3 );
			fSetArg( 3, a4 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2, typename tA3, typename tA4, typename tA5 >
	struct tFuiFuncCall< void ( tA1, tA2, tA3, tA4, tA5 ) > : public tFuncCallData< 5, 5 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
			fSetArgType<tA3>( 2 );
			fSetArgType<tA4>( 3 );
			fSetArgType<tA5>( 4 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2, const tA3& a3, const tA4& a4, const tA5& a5 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			fSetArg( 2, a3 );
			fSetArg( 3, a4 );
			fSetArg( 4, a5 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2, typename tA3, typename tA4, typename tA5, typename tA6 >
	struct tFuiFuncCall< void ( tA1, tA2, tA3, tA4, tA5, tA6 ) > : public tFuncCallData< 6, 6 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fCommonInit( );
			fSet( name, fui );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
			fSetArgType<tA3>( 2 );
			fSetArgType<tA4>( 3 );
			fSetArgType<tA5>( 4 );
			fSetArgType<tA6>( 5 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2, const tA3& a3, const tA4& a4, const tA5& a5, const tA6& a6 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			fSetArg( 2, a3 );
			fSetArg( 3, a4 );
			fSetArg( 4, a5 );
			fSetArg( 5, a6 );
			return fExecuteInternal( ); 
		}
	};

	template< typename tA1, typename tA2, typename tA3, typename tA4, typename tA5, typename tA6, typename tA7 >
	struct tFuiFuncCall< void ( tA1, tA2, tA3, tA4, tA5, tA6, tA7 ) > : public tFuncCallData< 7, 7 >
	{
		tFuiFuncCall( ) { fCommonInit( ); }
		tFuiFuncCall( const char* name, Iggy* fui ) 
		{ 
			fSet( name, fui );
			fCommonInit( );
		}

		void fCommonInit( )
		{
			fSetArgType<tA1>( 0 );
			fSetArgType<tA2>( 1 );
			fSetArgType<tA3>( 2 );
			fSetArgType<tA4>( 3 );
			fSetArgType<tA5>( 4 );
			fSetArgType<tA6>( 5 );
			fSetArgType<tA7>( 6 );
		}

		b32 fExecute( const tA1& a1, const tA2& a2, const tA3& a3, const tA4& a4, const tA5& a5, const tA6& a6, const tA7& a7 ) 
		{ 
			fSetArg( 0, a1 );
			fSetArg( 1, a2 );
			fSetArg( 2, a3 );
			fSetArg( 3, a4 );
			fSetArg( 4, a5 );
			fSetArg( 5, a6 );
			fSetArg( 6, a7 );
			return fExecuteInternal( ); 
		}
	};



	// !!!! Old deprecated function call style !!!!

	struct tVariableDataType
	{
		enum tVariableType
		{
			cTypeS32,
			cTypeF32,
			cTypeU32,
			cTypeB32,
			cTypeTString,
			cTypeLocalizedString,
			cTypeCount
		};

		tVariableDataType( ) 
			: mType( cTypeCount )
		{ }

		void fSetNumber( u32 n ) 
		{
			mType = cTypeU32;
			mValue.i = n;
		}

		void fSetNumber( s32 n )
		{
			mType = cTypeS32;
			mValue.i = n;
		}

		void fSetNumber( f32 n )
		{
			mType = cTypeF32;
			mValue.f = n;
		}

		void fSetBool( b32 b ) 
		{
			mType = cTypeB32;
			mValue.u = b;
		}

		void fSetStringUTF8( tStringPtr s ) 
		{
			mType = cTypeTString;
			mString = s;
		}

		void fSetStringUTF8( tFilePathPtr s )
		{
			mType = cTypeTString;
			mString = tStringPtr( s.fCStr( ) );
		}

		void fSetStringUTF16( const tLocalizedString& s ) 
		{
			mType = cTypeLocalizedString;
			mLocString = s;
		}

		union tValue 
		{
			s32		i;
			f32		f;
			u32     u;
		} mValue;

		tStringPtr mString;
		tLocalizedString mLocString;
		tVariableType mType;

	public:
		//convenience functions:
		static tVariableDataType fMakeNumber( u32 n ) 
		{
			tVariableDataType t;
			t.mType = cTypeU32;
			t.mValue.i = n;
			return t;
		}

		static tVariableDataType fMakeNumber( s32 n )
		{
			tVariableDataType t;
			t.mType = cTypeS32;
			t.mValue.i = n;
			return t;
		}

		static tVariableDataType fMakeNumber( f32 n )
		{
			tVariableDataType t;
			t.mType = cTypeF32;
			t.mValue.f = n;
			return t;
		}

		static tVariableDataType fMakeBool( b32 b ) 
		{
			tVariableDataType t;
			t.mType = cTypeB32;
			t.mValue.u = b;
			return t;
		}

		static tVariableDataType fMakeStringUTF8( tStringPtr s ) 
		{
			tVariableDataType t;
			t.mType = cTypeTString;
			t.mString = s;
			return t;
		}

		static tVariableDataType fMakeStringUTF8( tFilePathPtr s )
		{
			tVariableDataType t;
			t.mType = cTypeTString;
			t.mString = tStringPtr( s.fCStr( ) );
			return t;
		}

		static tVariableDataType fMakeStringUTF16( const tLocalizedString& s ) 
		{
			tVariableDataType t;
			t.mType = cTypeLocalizedString;
			t.mLocString = s;
			return t;
		}
	};

	struct tFuiFunctionCall : public tRefCounter
	{
		tFuiFunctionCall( const char* functionName )
			: mFunctionName( functionName )
			, mRetryOnFailure( false )
		{
			static u32 nextId = 0;
			mUniqueId = ++nextId;
		}

		b32 operator==( const tFuiFunctionCall& rhs ) const { return mUniqueId == rhs.mUniqueId; }

		tStringPtr mFunctionName;
		tGrowableArray< tVariableDataType > mValues;
		u32 mUniqueId;
		b32 mRetryOnFailure;
	};

	typedef tRefCounterPtr< tFuiFunctionCall > tFuiFunctionCallPtr;
}

#endif // __tFuiFunctionCall__