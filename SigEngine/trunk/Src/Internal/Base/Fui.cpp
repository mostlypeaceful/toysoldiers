#include "BasePch.hpp"
#include "Fui.hpp"
#include "tGameAppBase.hpp"
#include "tSwfFile.hpp"
#include "tFuiMemoryOptions.hpp"
#include "Allocators.hpp"

#	if defined( platform_pcdx9 )
#		include "iggy.h"
#		include "gdraw_d3d.h"
#	elif defined( platform_xbox360 )
#		include "iggy.h"
#       include "iggyperfmon.h"
#		include "gdraw_x360.h"
#	else
#		error "please setup the proper headers for Iggy for your platform"
#	endif

using namespace Sig::Input;

namespace Sig{ namespace Fui
{
	namespace
	{
		devvar( bool, Fui_ShowPerfmon, false );
		devvar( bool, Fui_GroupRendering, false );
		devvar( bool, Fui_IgnoreInputForPerfmon, false );
		devvar( bool, Fui_UseD3DStateBlocks, true );

#if defined( platform_xbox360 )
#if defined( sig_profile )
		HIGGYPERFMON iggy_perfmon = NULL;
#endif
#endif
		GDrawFunctions* gdraw_funcs = NULL;

		// Hand-roll removed Iggy functionality
		const char* IggyGetResultCodeString( IggyResult result )
		{
			switch( result )
			{
			#define TO_STRING( x ) case x: return #x
				TO_STRING( IGGY_RESULT_SUCCESS );

				// TO_STRING( IGGY_RESULT_Warning_None ); // Overlaps with IGGY_RESULT_SUCCESS above

				TO_STRING( IGGY_RESULT_Warning_Misc );
				TO_STRING( IGGY_RESULT_Warning_GDraw );
				TO_STRING( IGGY_RESULT_Warning_ProgramFlow );
				TO_STRING( IGGY_RESULT_Warning_Actionscript );
				TO_STRING( IGGY_RESULT_Warning_Graphics );
				TO_STRING( IGGY_RESULT_Warning_Font );
				TO_STRING( IGGY_RESULT_Warning_Timeline );
				TO_STRING( IGGY_RESULT_Warning_Library );

				TO_STRING( IGGY_RESULT_Warning_CannotSustainFrameRate );
				TO_STRING( IGGY_RESULT_Warning_ThrewException );

				// TO_STRING( IGGY_RESULT_Error_Threshhold ); // Overlaps with IGGY_RESULT_Error_Misc bellow

				TO_STRING( IGGY_RESULT_Error_Misc );
				TO_STRING( IGGY_RESULT_Error_GDraw );
				TO_STRING( IGGY_RESULT_Error_ProgramFlow );
				TO_STRING( IGGY_RESULT_Error_Actionscript );
				TO_STRING( IGGY_RESULT_Error_Graphics );
				TO_STRING( IGGY_RESULT_Error_Font );
				TO_STRING( IGGY_RESULT_Error_Create );
				TO_STRING( IGGY_RESULT_Error_Library );
				TO_STRING( IGGY_RESULT_Error_ValuePath );
				TO_STRING( IGGY_RESULT_Error_Audio );

				TO_STRING( IGGY_RESULT_Error_Internal );

				TO_STRING( IGGY_RESULT_Error_InvalidIggy );
				TO_STRING( IGGY_RESULT_Error_InvalidArgument );
				TO_STRING( IGGY_RESULT_Error_InvalidEntity );
				TO_STRING( IGGY_RESULT_Error_UndefinedEntity );

				TO_STRING( IGGY_RESULT_Error_OutOfMemory );
			#undef TO_STRING
			default:
				return "IGGY_RESULT_???";
			}
		}

		static void * RADLINK fIggyAlloc( void* user_data, size_t sizeRequested, size_t * outSizeAllocd )
		{
			void* data = Allocators::gFui->fAlloc( sizeRequested );
			if( data && outSizeAllocd )
				*outSizeAllocd = sizeRequested;
			return data;
		}
		static void RADLINK fIggyFree( void* user_data, void* ptr )
		{
			Allocators::gFui->fFree( ptr );
		}

		static void *RADLINK perf_malloc(void *handle, U32 size)
		{
			return fIggyAlloc(handle, size, NULL);
		}

		static void RADLINK perf_free(void *handle, void *ptr)
		{
			fIggyFree(handle, ptr);
		}

		static void RADLINK fWarningCallback(void *user_callback_data, Iggy *player, IggyResult code, const char* buffer )
		{
#ifdef sig_logging
			// totally ignore some really annoying messages
			if( code == IGGY_RESULT_Warning_CannotSustainFrameRate )
				return;

			// if we got here, it's a legit warning.
			if( Log::fFlagEnabled( Log::cFlagFui ) )
				log_warning( "IGGY: " << buffer );

			const char badFontErr[] = "Error InvalidSWF Tag length incorrect for tag 75";
			if( StringUtil::fStrStrI( buffer, badFontErr ) )
				log_warning( " Adobe Flash supports two kinds of fonts, 'Embedded' fonts and 'Device' fonts. Iggy supports only 'Embedded fonts'." << std::endl );
#endif//sig_logging
		}
		
		//Note: if 'trace("hello");' is in your AS3 then this func will get called twice. once with "hello" and once with "\n"
		static void RADLINK fTraceCallbackUTF8(void *user_callback_data, Iggy *player, char const *utf8_string, S32 length_in_bytes)
		{
			if( Log::fFlagEnabled( Log::cFlagFui ) && length_in_bytes == 1 && utf8_string[0] == '\n' )
				log_line( 0, "" );
			else
				log_output( Log::cFlagFui, utf8_string );
		}
        static void RADLINK fTraceCallbackUTF16(void *user_callback_data, Iggy *player, wchar_t const *utf16_String, S32 length_in_bytes)
        {
            log_output( Log::cFlagFui, utf16_String );
        }

		static void fLogAS3Func(IggyExternalFunctionCallUTF8* call)
		{
#ifdef sig_logging
			//if( !Log::fFlagEnabled( Log::cFlagFui ) )
				return;

			log_output( 0, "~~~IGGYFUNC CALLBACK: " << call->function_name.string );

			if( call->num_arguments )
			{
				log_output( 0, "  ARGS: " << call->num_arguments );
				for(s32 i = 0; i < call->num_arguments; ++i)
				{
					log_output( 0, " [" << i << "]: " );
					switch( call->arguments[i].type )
					{
					case IGGY_DATATYPE_boolean:
						log_output( 0, "{bool} " << call->arguments[i].boolval );
						break;
					case IGGY_DATATYPE_number:
						log_output( 0, "{number} " << call->arguments[i].number );
						break;
					case IGGY_DATATYPE_string_UTF8:
						{
							const std::string str( call->arguments[i].string8.string, call->arguments[i].string8.length );
							log_output( 0, "{string_utf8} " << str );
						}
						break;
					default:
						log_output( 0, "{unknown type}" );
					}
				}
			}
			log_output( 0, std::endl );
#endif//sig_logging
		}
		static rrbool RADLINK fAS3CallbackUTF8(void* user_data, Iggy* player, IggyExternalFunctionCallUTF8* call)
		{
			fLogAS3Func( call );

			const tStringPtr func_name( call->function_name.string );
			tFuiFuncParams params( tFuiSystem::fInstance( ).fConvertFromHandle( player ), call );

			// TODO, translator should be in the tFui returned by fConvertFromHandle, someone made this global when it shoudl be per swf/fui.
			tFuiSystem::fInstance( ).fTranslateAndExecute( func_name, params );

			return true; //always return true unless we want to cause an AS3 exception
		}
            
	}//unnamed namespace

	b32 fGroupRendering( )
	{
		return Fui_GroupRendering;
	}
	b32 fIgnoreFilteredInput( )
	{
		return Fui_IgnoreInputForPerfmon && Fui_ShowPerfmon;
	}

//-------------------- tFui


	tFuiValuePath::tFuiValuePath( )
	{ }

	tFuiValuePath::~tFuiValuePath( )
	{ }

	tFui::tGamepadTranslator::tGamepadTranslator( )
	{        
		fInsert( tGamepad::cButtonStart, IGGY_KEYCODE_ENTER );
		fInsert( tGamepad::cButtonSelect, IGGY_KEYCODE_SPACE );
		fInsert( tGamepad::cButtonA, IGGY_KEYCODE_A );
		fInsert( tGamepad::cButtonB, IGGY_KEYCODE_B );
		fInsert( tGamepad::cButtonX, IGGY_KEYCODE_X );
		fInsert( tGamepad::cButtonY, IGGY_KEYCODE_Y );
		fInsert( tGamepad::cButtonDPadRight, IGGY_KEYCODE_RIGHT );
		fInsert( tGamepad::cButtonDPadUp, IGGY_KEYCODE_UP );
		fInsert( tGamepad::cButtonDPadLeft, IGGY_KEYCODE_LEFT );
		fInsert( tGamepad::cButtonDPadDown, IGGY_KEYCODE_DOWN );
		fInsert( tGamepad::cButtonLStickRight, IGGY_KEYCODE_RIGHT );
		fInsert( tGamepad::cButtonLStickUp, IGGY_KEYCODE_UP );
		fInsert( tGamepad::cButtonLStickLeft, IGGY_KEYCODE_LEFT );
		fInsert( tGamepad::cButtonLStickDown, IGGY_KEYCODE_DOWN );
		fInsert( tGamepad::cButtonLShoulder, IGGY_KEYCODE_COMMA ); //aka '<'
		fInsert( tGamepad::cButtonLThumb, IGGY_KEYCODE_LEFTBRACKET ); //aka '['
		fInsert( tGamepad::cButtonLTrigger,IGGY_KEYCODE_SEMICOLON ); //aka ';'
		fInsert( tGamepad::cButtonLThumbMaxMag, IGGY_KEYCODE_BACKQUOTE ); //aka '`'
		//fInsert( tGamepad::cButtonLThumbMinMag,
		fInsert( tGamepad::cButtonRShoulder, IGGY_KEYCODE_PERIOD ); //aka '>'
		fInsert( tGamepad::cButtonRThumb, IGGY_KEYCODE_RIGHTBRACKET ); //aka ']'
		fInsert( tGamepad::cButtonRTrigger,IGGY_KEYCODE_QUOTE ); //aka '"'
		//fInsert( tGamepad::cButtonRThumbMaxMag,
		//fInsert( tGamepad::cButtonRThumbMinMag,

	}

    tFui::tKeyboardTranslator::tKeyboardTranslator( )
    {        
        fInsert( tKeyboard::cButtonEnter, IGGY_KEYCODE_ENTER );
        fInsert( tKeyboard::cButtonSpace, IGGY_KEYCODE_SPACE );
        fInsert( tKeyboard::cButtonA, IGGY_KEYCODE_A );
        fInsert( tKeyboard::cButtonB, IGGY_KEYCODE_B );
        fInsert( tKeyboard::cButtonX, IGGY_KEYCODE_X );
        fInsert( tKeyboard::cButtonY, IGGY_KEYCODE_Y );
        fInsert( tKeyboard::cButtonRight, IGGY_KEYCODE_RIGHT );
        fInsert( tKeyboard::cButtonUp, IGGY_KEYCODE_UP );
        fInsert( tKeyboard::cButtonLeft, IGGY_KEYCODE_LEFT );
        fInsert( tKeyboard::cButtonDown, IGGY_KEYCODE_DOWN );
        
        fInsert( tKeyboard::cButtonO, IGGY_KEYCODE_COMMA ); //aka '<'
        fInsert( tKeyboard::cButtonK, IGGY_KEYCODE_LEFTBRACKET ); //aka '['
        fInsert( tKeyboard::cButtonN,IGGY_KEYCODE_SEMICOLON ); //aka ';'
        fInsert( tKeyboard::cButtonHome, IGGY_KEYCODE_BACKQUOTE ); //aka '`'

        fInsert( tKeyboard::cButtonP, IGGY_KEYCODE_PERIOD ); //aka '>'
        fInsert( tKeyboard::cButtonL, IGGY_KEYCODE_RIGHTBRACKET ); //aka ']'
        fInsert( tKeyboard::cButtonM,IGGY_KEYCODE_QUOTE ); //aka '"'
    }

	tFui::tGamepadTranslator tFui::sGamepadTranslator;
    tFui::tKeyboardTranslator tFui::sKeyboardTranslator;

	tFui::tFui( const tResourcePtr& res, tFuiHandle fui )
		: mHandle( fui )
		, mVisible( true )
		, mRes( res )
	{
	}
	tFui::~tFui( )
	{
		sigassert( mHandle );
		if( mHandle )
		{
			IggyPlayerDestroy( (Iggy*)mHandle );
			mHandle = 0;
		}
	}
	b32 tFui::fInit( )
	{
		Iggy* swf = NULL;
		//init
		{
			profile_pix("IggyPlayerInitializeAndTickRS");
			sigassert( mHandle );
			//init swf
			swf = (Iggy*)mHandle;
			Time::tStopWatch sw;
			IggyPlayerInitializeAndTickRS( swf );
			const f32 ms = sw.fGetElapsedMs( );
			if( ms > 10 )
				log_line( 0, "IggyPlayerInitializeAndTickRS = " << mRes->fGetPath( ) << " took: " << std::fixed << std::setprecision( 2 ) << sw.fGetElapsedMs( ) << "ms" );
		}
		//start playing swf
		{
			profile_pix("IggyPlayerPlay");
			IggyPlayerPlay( swf );
		}
		return true;
	}
	tFuiHandle tFui::fHandle( ) const
	{
		return mHandle;
	}
	b32 tFui::fIsValid( ) const
	{
		return mHandle != 0;// && this;
	}
	void tFui::fEnd( )
	{
		sigassert( fIsValid( ) );
		if( !fIsValid( ) )
			return;

		tFuiSystem::fInstance( ).fRemove( tFuiPtr( this ) );
	}
	void tFui::fSendGamepadEvent( const Input::tGamepad::tButton& button, b32 isDown )
	{
		sigassert( fIsValid( ) );
		if( !fIsValid( ) )
			return;

		u32* kc = sGamepadTranslator.fFind( button );

		if( !kc )
		{
			log_warning( "Unable to find keyboard translation for " << button );
			return;
		}
		
		IggyEvent keyEvent;
		IggyKeyevent keyEventState = isDown ? IGGY_KEYEVENT_Down : IGGY_KEYEVENT_Up;
		IggyMakeEventKey(&keyEvent, keyEventState, *(IggyKeycode*)kc, IGGY_KEYLOC_Standard);

		IggyPlayerDispatchEventRS( (Iggy*)mHandle, &keyEvent, 0 );
	}

    void tFui::fSendKeyboardEvent( const Input::tKeyboard::tButton& button, b32 isDown )
    {
        sigassert( fIsValid( ) );
        if( !fIsValid( ) )
            return;

        u32* kc = sKeyboardTranslator.fFind( button );

        if( !kc )
        {
            log_warning( "Unable to find keyboard translation for " << button );
            return;
        }

        IggyEvent keyEvent;
        IggyKeyevent keyEventState = isDown ? IGGY_KEYEVENT_Down : IGGY_KEYEVENT_Up;
        IggyMakeEventKey(&keyEvent, keyEventState, *(IggyKeycode*)kc, IGGY_KEYLOC_Standard);

        IggyPlayerDispatchEventRS( (Iggy*)mHandle, &keyEvent, 0 );
    }

	b32 tFui::fCallFunction( const tFuiFunctionCall& functionCall )
	{
		profile_pix( "tFui::fCallFunction" );

		tGrowableArray< IggyDataValue > args;
		const tGrowableArray< tVariableDataType >& values = functionCall.mValues;
		args.fReserve( values.fCount( ) );
		IggyDataValue* arg = NULL;
		char * argStr = NULL;
        wchar_t * argLocStr = NULL;
		for( u32 i = 0; i < values.fCount( ); ++i )
		{
			switch( values[ i ].mType )
			{
			case tVariableDataType::cTypeF32:
				args.fPushBack( IggyDataValue( ) );
				arg = &args.fBack( );
				arg->type = IGGY_DATATYPE_number;
				arg->number = values[ i ].mValue.f;
				break;
			case tVariableDataType::cTypeS32:
				args.fPushBack( IggyDataValue( ) );
				arg = &args.fBack( );
				arg->type = IGGY_DATATYPE_number;
				arg->number = values[ i ].mValue.i;
				break;
            case tVariableDataType::cTypeU32:
                args.fPushBack( IggyDataValue( ) );
                arg = &args.fBack( );
                arg->type = IGGY_DATATYPE_number;
                arg->number = values[ i ].mValue.u;
                break;
            case tVariableDataType::cTypeB32:
                args.fPushBack( IggyDataValue( ) );
                arg = &args.fBack( );
                arg->type = IGGY_DATATYPE_boolean;
                arg->boolval = values[ i ].mValue.u ? 1 : 0;
                break;
			case tVariableDataType::cTypeTString:
				args.fPushBack( IggyDataValue( ) );
				arg = &args.fBack( );
				arg->type = IGGY_DATATYPE_string_UTF8;
				argStr = const_cast< char* >( values[ i ].mString.fCStr( ) );
				arg->string8.string = argStr;
				arg->string8.length = strlen( argStr );
				break;
            case tVariableDataType::cTypeLocalizedString:
                args.fPushBack( IggyDataValue( ) );
                arg = &args.fBack( );
                arg->type = IGGY_DATATYPE_string_UTF16;
                argLocStr = const_cast< wchar_t* >( values[ i ].mLocString.fCStr( ) );
                arg->string16.string = argLocStr;
                arg->string16.length = wcslen( argLocStr );
                break;
			default:
				break;
			}
		}

		const tStringPtr& name = functionCall.mFunctionName;
		const IggyName functionName = IggyPlayerCreateFastNameUTF8( (Iggy*)mHandle, name.fCStr( ), name.fLength( ) );
		const IggyResult result = IggyPlayerCallFunctionRS( (Iggy*)mHandle, NULL, functionName, args.fCount( ), args.fBegin( ) );
		if( result != IGGY_RESULT_SUCCESS )
		{
			if( Log::fFlagEnabled( Log::cFlagFui ) )
				log_warning( "tFui::fCallFunction [" << name << "] failed with " << IggyGetResultCodeString( result ) );
			return false;
		}

		return true; 
	}
	b32 tFui::fGetPath( const char* name, tFuiValuePath& pathOut ) const
	{
		sigcheckfail( fIsValid( ), return false );

		//Get root of swf
		IggyValuePath* root = IggyPlayerRootPath( (Iggy*)mHandle );
		log_sigcheckfail( root, "failed to access root of swf file. is something corrupted? consult Iggy docs...", return false );

		tGrowableArray< std::string > frags;
		StringUtil::fSplit( frags, name, "." );

		if( frags.fCount( ) == 0 )
			return false;

		// array cannot resize or data is invalidated.
		pathOut.mStorage.fSetCount( 0 );
		pathOut.mStorage.fSetCapacity( frags.fCount( ) );

		for( u32 i = 0; i < frags.fCount( ); ++i )
		{
			IggyValuePath& current = pathOut.mStorage.fPushBack( );

			//get access to display object
			const rrbool success = IggyValuePathMakeNameRef( &current, root, frags[ i ].c_str( ) );
			log_sigcheckfail( success, "failed to find display object '" << name << "' in flash file", return false );

			root = &current;
		}

		return true;
	}

	void tFui::fGetUserData( const char * pathName, void * * userData )
	{
		tFuiValuePath path;
		if( fGetPath( pathName, path ) )
			fGetUserData( path, userData );
		else *userData = NULL;
	}

	void tFui::fGetUserData( const tFuiValuePath& path, void * * userData )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const IggyResult result = IggyValueGetUserDataRS( path.fValue( ), userData );
		log_sigcheckfail( result == IGGY_RESULT_SUCCESS, "tFui::fGetUserData: IggyValueGetUserDataRS failed for some unknown reason. consult the Iggy docs. " << this << " - " << IggyGetResultCodeString( result ), *userData = NULL );
	}

	void tFui::fSetUserData( const char * pathName, void const * userData )
	{
		tFuiValuePath path;
		fGetPath( pathName, path );
		fSetUserData( path, userData );
	}

	void tFui::fSetUserData( const tFuiValuePath& path, void const * userData )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const rrbool success = IggyValueSetUserDataRS( path.fValue( ), userData );
		log_assert( success, "tFui::fSetUserData: IggyValueSetUserDataRS failed for some unknown reason. consult the Iggy docs. " << this );
	}

	void tFui::fSetFloat( const tFuiValuePath& path, const f32 val )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const rrbool success = IggyValueSetF64RS( path.fValue( ), 0, NULL, val );
		log_assert( success, "tFui::fSetFloat: IggyValueSetF64RS failed for some unknown reason. consult the Iggy docs. " << this );
	}

	void tFui::fSetS32( const tFuiValuePath& path, const s32 val )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const rrbool success = IggyValueSetS32RS( path.fValue( ), 0, NULL, val );
		log_assert( success, "tFui::fSetS32: IggyValueSetS32RS failed for some unknown reason. consult the Iggy docs. " << this );
	}

	void tFui::fSetString( const tFuiValuePath& path, const std::string& str )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const rrbool success = IggyValueSetStringUTF8RS( path.fValue( ), 0, NULL, str.c_str( ), str.length( ) );
		log_assert( success, "tFui::fSetString: IggyValueSetStringUTF8RS failed for some unknown reason. consult the Iggy docs. " << this );
	}

	void tFui::fSetWString( const tFuiValuePath& path, const std::wstring& wstr )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );
		const rrbool success = IggyValueSetStringUTF16RS( path.fValue( ), 0, NULL, wstr.c_str( ), wstr.length( ) );
		log_assert( success, "tFui::fSetWString: IggyValueSetStringUTF16RS failed for some unknown reason. consult the Iggy docs. " << this );
	}

	void tFui::fSetArray( const tFuiValuePath& path, const tGrowableArray<tStringPtr>& strings )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );

		//Get pointer to array
		IggyValuePath elem;
		IggyValuePathMakeArrayRef( &elem, path.fValue( ), 0 );

		//loop over each element in array and set values
		for(u32 i = 0; i < strings.fCount( ); ++i)
		{
			IggyValuePathSetArrayIndex( &elem, i );
			const rrbool success = IggyValueSetStringUTF8RS( &elem, 0, NULL, strings[ i ].fCStr( ), strings[ i ].fLength( ) );
			log_sigcheckfail( success, "tFui::fSetArray: IggyValueSetStringUTF8RS failed for some unknown reason. consult the Iggy docs. " << this, return );
		}
	}
	void tFui::fSetArray( const tFuiValuePath& path, const tGrowableArray<u32>& values )
	{
		sigassert( path.fValue( ) && "Need to specify a path!" );

		//Get pointer to array
		IggyValuePath elem;
		IggyValuePathMakeArrayRef( &elem, path.fValue( ), 0 );

		//loop over each element in array and set values
		for(u32 i = 0; i < values.fCount( ); ++i)
		{
			IggyValuePathSetArrayIndex( &elem, i );

			const rrbool success = IggyValueSetU32RS( &elem, 0, NULL, values[ i ] );
			log_sigcheckfail( success, "tFui::fSetArray: IggyValueSetU32RS failed for some unknown reason. consult the Iggy docs. " << this, return );
		}
	}

	//-------------------- tFuiFuncParams
	tFuiFuncParams::tFuiFuncParams( tFuiPtr fui, tFuiFunctionHandle params )
		: mFui( fui )
		, mParams( params )
		, mIndex( 0 )
	{}
	b32 tFuiFuncParams::fIsValid( ) const
	{
		return ( mFui ) && ( mParams != 0 ) && ( mIndex != -1 );
	}
	const tFuiPtr& tFuiFuncParams::fFui( ) const
	{
		return mFui;
	}
	u32 tFuiFuncParams::fCount( ) const
	{
		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;
		return params->num_arguments;
	}
	b32 tFuiFuncParams::fValidate( u32 expectedType )
	{
		sigassert( fIsValid( ) );
		if( !fIsValid( ) )
			return false;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;
		if( mIndex >= params->num_arguments )
		{
			const std::string funcName( params->function_name.string, params->function_name.length );
			log_warning( ((void*)this) << " " << funcName << " too few paramaters passed from flash file to C++" );
			sigassert( mIndex < params->num_arguments );			
			mIndex = -1;
			return false;
		}
		if( params->arguments[ mIndex ].type != expectedType )
		{
			const std::string funcName( params->function_name.string, params->function_name.length );
			log_warning( ((void*)this) << " " << funcName << " invalid type passed in for param " << mIndex << ". Expected: " << expectedType << ", Got: " << params->arguments[ mIndex ].type );
			sigassert( params->arguments[ mIndex ].type == expectedType );
			mIndex = -1;
			return false;
		}
		return true;
	}
	void tFuiFuncParams::fGet( u32& val )
	{
		if( !fValidate( IGGY_DATATYPE_number ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = (u32)params->arguments[ mIndex ].number;
		++mIndex;
	}
	void tFuiFuncParams::fGet( s32& val )
	{
		if( !fValidate( IGGY_DATATYPE_number ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = (s32)params->arguments[ mIndex ].number;
		++mIndex;
	}
	void tFuiFuncParams::fGet( f32& val )
	{
		if( !fValidate( IGGY_DATATYPE_number ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = (f32)params->arguments[ mIndex ].number;
		++mIndex;
	}
	void tFuiFuncParams::fGet( bool & val )
	{
		 if( !fValidate( IGGY_DATATYPE_boolean ) )
            return;

        IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;
        val = params->arguments[ mIndex ].boolval ? true : false;
        ++mIndex;
	}
	void tFuiFuncParams::fGet( const char*& val )
	{
		if( !fValidate( IGGY_DATATYPE_string_UTF8 ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = params->arguments[ mIndex ].string8.string;
		++mIndex;
	}
    void tFuiFuncParams::fGet( const wchar_t*& val )
    {
        if( !fValidate( IGGY_DATATYPE_string_UTF16 ) )
            return;

        IggyExternalFunctionCallUTF16* params = (IggyExternalFunctionCallUTF16*)mParams;
        val = params->arguments[ mIndex ].string16.string;
        ++mIndex;
	}
	void tFuiFuncParams::fGet( std::string& val )
	{
		if( !fValidate( IGGY_DATATYPE_string_UTF8 ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = std::string( params->arguments[ mIndex ].string8.string, params->arguments[ mIndex ].string8.length );
		++mIndex;
	}
	void tFuiFuncParams::fGet( tStringPtr& val )
	{
		if( !fValidate( IGGY_DATATYPE_string_UTF8 ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = tStringPtr( params->arguments[ mIndex ].string8.string );
		++mIndex;
	}
	void tFuiFuncParams::fGet( tFilePathPtr& val )
	{
		if( !fValidate( IGGY_DATATYPE_string_UTF8 ) )
			return;

		IggyExternalFunctionCallUTF8* params = (IggyExternalFunctionCallUTF8*)mParams;		
		val = tFilePathPtr( params->arguments[ mIndex ].string8.string );
		++mIndex;
	}
    
    void tFuiFuncParams::fGetBool( b32& val )
    {
        bool tempVal; fGet( tempVal );
		val = tempVal ? true : false;
    }

	//-------------------- tFuiSystem
	const u32 tFuiSystem::cFontFlag_None = IGGY_FONTFLAG_none;
	const u32 tFuiSystem::cFontFlag_Bold = IGGY_FONTFLAG_bold;
	const u32 tFuiSystem::cFontFlag_Italic = IGGY_FONTFLAG_italic;

	tFuiSystem::tFuiSystem( )
		: mTicking( false )
		, mRenderStateBlock( NULL )
	{
	}
	tFuiSystem::~tFuiSystem( )
	{
		if( mRenderStateBlock )
			mRenderStateBlock->Release( );
		mRenderStateBlock = NULL;
	}

	static const u32 CHAR_BUFFER_SIZE = 255;
	GDrawTexture* RADLINK TextureSubstitutionCreateCallback(void *user_callback_data, char *texture_name, S32 *width, S32 *height, void **destroy_callback_data)
	{
		u32 texWidth, texHeight = 0;
		Gfx::tTextureFile::tPlatformHandle texHandle = tFuiSystem::fInstance( ).fGetTexture( texture_name, &texWidth, &texHeight );
		if( !texHandle )
			return NULL;

		*destroy_callback_data = (void *) texHandle; // store handle so we can get it easily
#if defined( platform_pcdx9 )
		GDrawTexture * gdraw_tex = gdraw_D3D_WrappedTextureCreate((IDirect3DTexture9*)texHandle);
#elif defined( platform_xbox360 )
		GDrawTexture * gdraw_tex = gdraw_x360_WrappedTextureCreate((IDirect3DTexture9*)texHandle);
#else
#error "please define iggy gdraw WrappedTextureCreate for your platform"
#endif

		// if the attempt to wrap fails (would only happen if GDraw is out of GDrawTexture handles)
		if( !gdraw_tex )
		{
			tFuiSystem::fInstance( ).fReleaseTexture( texHandle );
			return NULL; // fail
		}

		// Success!
		*width = texWidth;
		*height = texHeight;
		return gdraw_tex;
	}

	void RADLINK TextureSubstitutionDestroyCallback(void *user_callback_data, void *destroy_callback_data, GDrawTexture *handle)
	{
		//log_line( Log::cFlagFui, "TextureSubstitutionDestroyCallback Handle - " << std::hex << handle );

#if defined( platform_pcdx9 )
		gdraw_D3D_WrappedTextureDestroy(handle);
#elif defined( platform_xbox360 )
		gdraw_x360_WrappedTextureDestroy(handle);
#else
#error "please define gdraw WrappedTextureDestroy for your platform"
#endif
		tFuiSystem::fInstance( ).fReleaseTexture( (Gfx::tTextureFile::tPlatformHandle)destroy_callback_data );
	}

	rrbool RADLINK TranslationFunctionCallback( void * callback_data, IggyStringUTF8 * src, IggyStringUTF8 * dest )
	{
		src->string[ src->length ] = 0;
		tLocalizedString locString = tGameAppBase::fInstance( ).fLocString( tStringPtr( src->string ) );

		if( !locString.fLength( ) )
		{
			dest->string = NEW char[ 0 ];
			dest->length = 0;
			return true;
		}

		const wchar_t *orig = locString.fCStr( );

		// Convert to a char*
		size_t nsize = WideCharToMultiByte(CP_UTF8, 0, orig, locString.fLength( ), NULL, 0, NULL, NULL );
		char * nstring = NEW char[ nsize ];
		WideCharToMultiByte(CP_UTF8, 0, orig, locString.fLength( ), nstring, nsize, NULL, NULL );

		dest->string = nstring;
		dest->length = nsize;
		return true;
	}

	void RADLINK TranslationFreeFunctionCallback( void * callback_data, void * data, S32 length )
	{
		delete[ ] data;
	}

	rrbool RADLINK TextfieldTranslationFunctionCallback( void * callback_data, IggyStringUTF8 * src, IggyStringUTF8 * dest, IggyTextfieldInfo * textfield )
	{
		src->string[ src->length ] = 0;
		tStringPtr translationString( src->string );
		log_line( 0, "translating string " << translationString );
		if( tGameAppBase::fInstance( ).fLocStringExists( translationString ) )
		{
			tLocalizedString locString = tGameAppBase::fInstance( ).fLocString( translationString );

			const wchar_t *orig = locString.fCStr( );

			// Convert to a char*
			size_t origsize = wcslen(orig) + 1;
			const size_t newsize = 100;
			size_t convertedChars = 0;
			char nstring[newsize];
			wcstombs_s(&convertedChars, nstring, origsize, orig, _TRUNCATE);

			dest->string = nstring;
			dest->length = convertedChars;
			return true;
		}
		return false;
	}

	Gfx::tTextureFile::tPlatformHandle tFuiSystem::fGetTexture( const char * texture, u32 * outWidth, u32 * outHeight)
	{
		return mResourceProvider.fTextureLookup( texture, outWidth, outHeight );
	}

	void tFuiSystem::fPermaLoadTexture( const tFilePathPtr& filename )
	{
		const b32 permaLoad = true;
		mResourceProvider.fAddTextureResource( tResourceId::fMake<Gfx::tTextureFile>( filename ), permaLoad );
	}

	void tFuiSystem::fReleaseTexture( const Gfx::tTextureFile::tPlatformHandle& handle )
	{
		mResourceProvider.fReleaseTexture( handle );
	}

	tFuiMemoryOptions gFuiMemoryOptions;

	devvar( bool, Fui_Limits_ForceUseDefaults, false );
	devvarptr( u32, Fui_Limits_RenderTargetHandles,	gFuiMemoryOptions.mRenderTargets.mNumHandles );
	devvarptr( u32, Fui_Limits_RenderTargetBytes,	gFuiMemoryOptions.mRenderTargets.mNumBytes );
	devvarptr( u32, Fui_Limits_TextureHandles,		gFuiMemoryOptions.mTextures.mNumHandles );
	devvarptr( u32, Fui_Limits_TextureBytes,		gFuiMemoryOptions.mTextures.mNumBytes );
	devvarptr( u32, Fui_Limits_VertexBufferHandles,	gFuiMemoryOptions.mVertexBuffers.mNumHandles );
	devvarptr( u32, Fui_Limits_VertexBuffereBytes,	gFuiMemoryOptions.mVertexBuffers.mNumBytes );

	void tFuiSystem::fInit( const Gfx::tDevicePtr& device )
	{
		fInit( device, gFuiMemoryOptions );
	}

	void tFuiSystem::fInit( const Gfx::tDevicePtr& device, const tFuiMemoryOptions & memOpts_ )
	{
		const tFuiMemoryOptions& memOpts = Fui_Limits_ForceUseDefaults ? gFuiMemoryOptions : memOpts_;

		sigassert( !mDevice ); //we should only init once
		mDevice = device;

		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextFui ) );

		//Init iggy + debug funcs
		IggyAllocator allocator = {0};
		allocator.mem_alloc = &fIggyAlloc;
		allocator.mem_free = &fIggyFree;
		IggyInit(&allocator);
		IggySetWarningCallback(fWarningCallback, 0);
		IggySetTraceCallbackUTF8(fTraceCallbackUTF8, 0);
        IggySetTraceCallbackUTF16(fTraceCallbackUTF16, 0);
		IggySetAS3ExternalFunctionCallbackUTF8(fAS3CallbackUTF8, 0);
        //IggySetAS3ExternalFunctionCallbackUTF16(fAS3CallbackUTF16, 0);
		IggySetTextureSubstitutionCallbacksUTF8(
			TextureSubstitutionCreateCallback,
			TextureSubstitutionDestroyCallback,
			NULL);
		//IggySetLoadtimeTranslationFunctionUTF8( TranslationFunctionCallback, NULL, NULL, NULL );
		//IggySetTextfieldTranslationFunctionUTF8( TextfieldTranslationFunctionCallback, NULL, NULL, NULL );
		IggySetRuntimeTranslationFunctionUTF8( TranslationFunctionCallback, NULL, TranslationFreeFunctionCallback, NULL );

#if defined( platform_xbox360 )
#if defined( sig_profile )
		iggy_perfmon = IggyPerfmonCreate(perf_malloc, perf_free, NULL);

		IggyInstallPerfmon(iggy_perfmon);

		sigassert( iggy_perfmon && "Couldn't init Iggy Perfmon!" );
#endif
#endif

		//Setup render device
		IDirect3DDevice9* d3d = mDevice->fGetDevice( );

#if defined( platform_pcdx9 )
		gdraw_funcs = gdraw_D3D_CreateContext( d3d, 0, 0 );
#elif defined( platform_xbox360 )
		
		// Set up resource memory
		struct 
		{
			gdraw_x360_resourcetype type;
			S32 num_handles;
			S32 num_bytes;
		} pools[] = {
			{ GDRAW_X360_RESOURCE_rendertarget, memOpts.mRenderTargets.mNumHandles,  memOpts.mRenderTargets.mNumBytes },
			{ GDRAW_X360_RESOURCE_texture, memOpts.mTextures.mNumHandles, memOpts.mTextures.mNumBytes },
			{ GDRAW_X360_RESOURCE_vertexbuffer, memOpts.mVertexBuffers.mNumHandles,  memOpts.mVertexBuffers.mNumBytes },
		};

		const DWORD attribs = MAKE_XALLOC_ATTRIBUTES(0,0,0,0,0,XALLOC_PHYSICAL_ALIGNMENT_64,0,0,XALLOC_MEMTYPE_PHYSICAL);

		for( u32 i = 0; i < array_length( pools ); ++i ) 
		{
			//void *ptr = XPhysicalAlloc(pools[i].num_bytes, MAXULONG_PTR, 0, PAGE_READWRITE|PAGE_WRITECOMBINE|MEM_LARGE_PAGES);
			void* ptr = tApplication::fInstance( ).fXMemAlloc( pools[i].num_bytes, attribs ); 
			sigassert( ptr );
			gdraw_x360_SetResourceMemory(pools[i].type, pools[i].num_handles, ptr, pools[i].num_bytes);
		}

		gdraw_funcs = gdraw_x360_CreateContext( d3d, 0, 0 );
#else
#error "please define iggy version of create context for your platform"
#endif
		sigassert(gdraw_funcs);
		IggySetGDraw(gdraw_funcs);

		Memory::tHeap::fResetVramContext( );
	}
	void tFuiSystem::fShutdown( )
	{
		mDevice.fRelease( );
		mPlayingSwfs.fSetCount( 0 );
		IggyShutdown( );
#if defined( platform_xbox360 )
#if defined( sig_profile )
		IggyPerfmonDestroy(iggy_perfmon, gdraw_funcs);
#endif
#endif
	}
	//1) Block until file is loaded
	//2) Convert to Iggy format
	//3) Add to list to begin playing on tFuiSystem::fTick()
	//3) Unload file
	tFuiPtr tFuiSystem::fLoadAndPlay( const tFilePathPtr& filename )
	{
		profile_pix( "tFuiSystem::fLoadAndPlay" );

		//Load file
		tResourceId id = tResourceId::fMake< tSwfFile >( filename );
		tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( id, this );
		if( !res->fLoaded( ) )
		{
			log_warning( "failed loading " << res->fAbsolutePhysicalPath( ) );
			sigassert( res->fLoaded( ) );
			return tFuiPtr( );
		}

		tFuiPtr retval = fPlay( res );

		// Uncommenting the below line fixes our graphical issues with Iggy..
		//IggyPlayerSetAntialiasing((Iggy*)retval->mHandle, IGGY_ANTIALIASING_FontsAndLinesOnly);
		
		return retval;
	}
	void tFuiSystem::fLoad( const tFilePathPtr& filename )
	{
		tFuiPtr fui = fLoadInternal( filename );
		mLoadedSwfs.fPushBack( fui );
		b32 okay = fui->fInit( );
		sigassert( okay );
	}
	//file should be loaded before requesting it to be played because we need that data to parse for iggy!
	tFuiPtr tFuiSystem::fPlay( const tResourcePtr& res )
	{
		profile_pix( "tFuiSystem::fPlay" );

		tFuiPtr fui;
		Iggy* swf = 0;

		//see if we've already created the fui in memory
		for( u32 i = 0; i < mLoadedSwfs.fCount( ); ++i )
		{
			if( mLoadedSwfs[i]->fResource( ) == res )
			{
				fui = mLoadedSwfs[i];
				mLoadedSwfs.fErase( i );
				swf = (Iggy*)fui->fHandle( );
				break;
			}
		}

		const b32 needsInit = swf == NULL;

		if( !fui )
		{
			fui = fLoadInternal( res->fGetPath( ) );
			swf = (Iggy*)fui->fHandle( );
		}
		sigcheckfail( fui, return tFuiPtr( ) );

		//toss it into the list of playing files
		mPlayingSwfs.fPushBack( fui );
		if( needsInit )
		{
			b32 okay = fui->fInit( );
			sigassert( okay );
		}
		sigassert( fui->fIsValid( ) );

		return fui;
	}
	tFuiPtr tFuiSystem::fLoadInternal( const tFilePathPtr& filename )
	{
		profile_pix( "tFuiSystem::fLoadInternal" );
		tResourceId id = tResourceId::fMake< tSwfFile >( filename );
		tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( id, this );

		//check that it is loaded
		log_sigcheckfail( res->fLoaded( ), res->fAbsolutePhysicalPath( ) << " is not fully loaded yet. cannot play", return tFuiPtr( ) );

		//make sure resource is valid
		tSwfFile* swfFile = res->fCast<tSwfFile>( );
		log_sigcheckfail( swfFile, res->fAbsolutePhysicalPath( ) << " is not a valid tSwfFile. cannot play", return tFuiPtr( ) );

		Iggy* swf = 0;
		{
			profile_pix( "IggyPlayerCreateFromMemory" );
			Time::tStopWatch sw;
			swf = IggyPlayerCreateFromMemory( swfFile->fData( ).fBegin( ), swfFile->fData( ).fCount( ), 0 );
			const f32 ms = sw.fGetElapsedMs( );
			if( ms > 10 )
				log_line( 0, "IggyPlayerCreateFromMemory = " << res->fGetPath( ) << " took: " << std::fixed << std::setprecision( 2 ) << sw.fGetElapsedMs( ) << "ms" );
		}
		log_sigcheckfail( swf, "Iggy failed IggyPlayerCreateFromMemory() in file: " << res->fAbsolutePhysicalPath( ), return tFuiPtr( ) );

		//let the file know how big our output buffer is. This will pretty much always be the screen back buffer
		{
			profile_pix("IggyPlayerSetDisplaySize");
			IggyPlayerSetDisplaySize( swf, mDevice->fCreationPresentParams( ).BackBufferWidth, mDevice->fCreationPresentParams( ).BackBufferHeight );
		}

		tFuiPtr fui( NEW_TYPED( tFui )( res, swf ) );
		return fui;
	}
	void tFuiSystem::fRemove( tFuiPtr fui )
	{
		sigassert( fui );
		if( !mPlayingSwfs.fFindAndErase( fui ) )
		{
			log_warning( "Failed to find and erase " << fui->fHandle( ) << " could this be double-deleted??" );
		}
	}
	void tFuiSystem::fRegisterFunc( const char* name_, tFuiCallback cb )
	{
		const tStringPtr name( name_ );
		if( mFuncTranslator.fFind( name ) )
			log_warning( "FUI function already registered! " << name );
		mFuncTranslator.fInsert( name, cb );
	}
	void tFuiSystem::fUnregisterFunc( const char* name_ )
	{
		const tStringPtr name( name_ );
		mFuncTranslator.fRemove( name );
	}
	void tFuiSystem::fTranslateAndExecute( const tStringPtr& name, tFuiFuncParams& params )
	{
		tFuiCallback* cb = mFuncTranslator.fFind( name );
		if( !cb )
		{
			log_warning( "~~~FUI - function " << name << " not registered with tFuiSystem" );
			return;
		}
		(*cb)( params );
	}
	tFuiPtr tFuiSystem::fConvertFromHandle( tFuiHandle fui )
	{
		for(u32 i = 0; i < mLoadedSwfs.fCount( ); ++i)
		{
			if( mLoadedSwfs[ i ]->fHandle( ) == fui )
				return mLoadedSwfs[ i ];
		}
		for(u32 i = 0; i < mPlayingSwfs.fCount( ); ++i)
		{
			if( mPlayingSwfs[ i ]->fHandle( ) == fui )
				return mPlayingSwfs[ i ];
		}
		return tFuiPtr( );
	}
	void tFuiSystem::fInstallTrueTypeFont( const tStringPtr& name, const void* fontBits, u32 fontFlags )
	{
		IggyFontInstallTruetypeUTF8
			( const_cast<void*>( fontBits )
			, IGGY_TTC_INDEX_none
			, const_cast<char*>( name.fCStr( ) )
			, name.fLength( )
			, fontFlags );
	}
	void tFuiSystem::fUninstallFont( const tStringPtr& name, u32 fontFlags )
	{
		IggyFontRemoveUTF8( const_cast<char*>( name.fCStr( ) ), name.fLength( ), fontFlags );
	}
	void tFuiSystem::fTick( )
	{
		profile_pix( "tFuiSystem::fTick" );
		mTicking = true;

		//Update all Swfs. This may take a while...
		for(u32 i = 0; i < mPlayingSwfs.fCount( ); ++i)
		{
			sigassert( mPlayingSwfs[ i ]->fIsValid( ) );
			Iggy* swf = (Iggy*)mPlayingSwfs[ i ]->fHandle( );
			while( IggyPlayerReadyToTick( swf ) )
			{
				IggyPlayerTickRS( swf );
			}
		}
		mTicking = false;
	}

	void tFuiSystem::fRender( const tFuiPtr& swf )
	{
		profile_pix( "tFuiSystem::fRender" );
		profile( cProfilePerfRenderFUI );

# ifndef target_tools
		if( tGameAppBase::fInstance( ).fRootHudCanvas( ).fCanvas( )->fParentIsInvisible( ) )
			return;
# endif

		// Setup Iggy for rendering.
		IDirect3DDevice9* d3d = mDevice->fGetDevice( );
		if( Fui_UseD3DStateBlocks )
		{
			if( !mRenderStateBlock )
				d3d->CreateStateBlock( D3DSBT_ALL, &mRenderStateBlock );
			HRESULT hr = mRenderStateBlock->Capture( );
			sigassert( SUCCEEDED( hr ) );
		}

#if defined( platform_pcdx9 )
		IDirect3DSurface9 *fb, *zb;
		d3d->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &fb );
		sigassert( fb );
		d3d->GetDepthStencilSurface(&zb);
		sigassert( zb );
		gdraw_D3D_SetTileOrigin( fb, zb, 0, 0 );
#elif defined( platform_xbox360 )
		sigassert( !tGameAppBase::fInstance( ).fScreen( )->fScreenSpaceRenderToTexture( )->fFailed( ) );
		IDirect3DSurface9 *fb, *zb;
		DWORD tileSize = 0;
		//d3d->GetBackBuffer( 0, 0, 0, &fb );
		fb = tGameAppBase::fInstance( ).fScreen( )->fScreenSpaceRenderToTexture( )->fRenderTarget( )->fGetSurface( );
		sigassert( fb );
		tileSize += XGSurfaceSize( 1280, 720, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE );
		//d3d->GetDepthStencilSurface(&zb);
		zb = tGameAppBase::fInstance( ).fScreen( )->fScreenSpaceRenderToTexture( )->fDepthTarget( )->fGetSurface( );
		sigassert( zb );
		tileSize += XGSurfaceSize( 1280, 720, D3DFMT_D24S8, D3DMULTISAMPLE_NONE );

		gdraw_x360_SetTileOrigin( fb, zb, 0, 0, tileSize, GPU_EDRAM_TILES - tileSize );
#else
#error "Please setup begin render Iggy code for your platform"
#endif

		if( fGroupRendering( ) )
		{
			sigassert( !swf );
			for( u32 i = 0; i < mPlayingSwfs.fCount( ); ++i )
			{
				IggyPlayerDraw( (Iggy*)mPlayingSwfs[ i ]->fHandle( ) );
			}
		}
		else
		{
			// Render the SWF.
			if( swf->fVisible( ) )
			{
				sigassert( swf->fIsValid( ) );
				IggyPlayerDraw( (Iggy*)swf->fHandle( ) );
			}
		}

		// Finish.
#if defined( platform_pcdx9 )
		gdraw_D3D_NoMoreGDrawThisFrame( );
		fb->Release( );
		zb->Release( );
#elif defined( platform_xbox360 )
		gdraw_x360_NoMoreGDrawThisFrame();
		//fb->Release( );
		//zb->Release( );
#else
#error "Please setup finish render Iggy code for your platform
#endif

		if( Fui_UseD3DStateBlocks )
		{
			HRESULT hr = mRenderStateBlock->Apply( );
			sigassert( SUCCEEDED( hr ) );
		}
	}

	void tFuiSystem::fOnRenderComplete( )
	{
#if defined( platform_xbox360 ) && defined( sig_profile )

		if( Fui_ShowPerfmon )
		{
			XINPUT_STATE xi_pad;
			XInputGetState( 0, &xi_pad );
			IggyPerfmonPad pm_pad;
			IggyPerfmonPadFromXInputStatePointer( pm_pad, &xi_pad );
		
			IggyPerfmonTickAndDraw( iggy_perfmon, gdraw_funcs, &pm_pad,
				0, 0, 1280, 720 );     // perfmon draw area in window coords
		}
#endif
	}

}}//Sig::Fui
