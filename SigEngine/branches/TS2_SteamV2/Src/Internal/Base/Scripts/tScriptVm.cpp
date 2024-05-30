#include "BasePch.hpp"
#include "tScriptVm.hpp"
#include "tScriptFile.hpp"
#include "Memory/tHeap.hpp"

namespace Sig
{
	namespace
	{
		struct tResetSqStack
		{
			HSQUIRRELVM mSqvm;
			s32			mTop;
			tResetSqStack( HSQUIRRELVM vm ) : mSqvm( vm ), mTop( sq_gettop( vm ) ) { }
			~tResetSqStack( ) { sq_settop( mSqvm, mTop ); }
		};
#define reset_sq_stack( ) tResetSqStack _reset_sq_stack_( mSq );

#ifdef target_game

//#define use_own_heap
		Memory::tAllocStamp cSQStamp( __FILE__, __LINE__ );

		void* fSqMalloc( u32 numBytes )
		{
			profile_mem( cProfileMemScript, numBytes );
#ifdef use_own_heap
			Memory::tAllocStamp stamp = cSQStamp;
			stamp.mSize = numBytes;
			return Memory::tHeap::fInstance( ).fAlloc( numBytes, stamp );
#else
			return malloc( numBytes );
#endif
		}
		void fSqFree( void* oldMem, u32 numBytes )
		{
			profile_mem( cProfileMemScript, -(s32)numBytes );
#ifdef use_own_heap
			Memory::tHeap::fInstance( ).fFree( oldMem );
#else
			return free( oldMem );
#endif
		}
		void* fSqRealloc( void* oldMem, u32 oldSize, u32 numBytes )
		{
			void* newAddr = fSqMalloc( numBytes );

			if( oldMem )
				memcpy( newAddr, oldMem, fMin( oldSize, numBytes ) );

			fSqFree( oldMem, oldSize );
			return newAddr;
		}

		define_static_function( fSqSetAllocFuncs )
		{
			sq_setallocfuncs( fSqMalloc, fSqRealloc, fSqFree );
			Sqrat::ErrorHandling::Enable( true );
		}
#endif//target_game

		u32 gCompileErrorLineNo=0;
		u32 gCompileErrorColumnNo=0;

		static void fIndent( u32 numTimes, std::stringstream& out )
		{
			for( u32 i = 0; i < numTimes; ++i )
				out <<  "\t";
		}

		static void fSqPrintVariables( HSQUIRRELVM v, u32 level, std::stringstream& out )
		{
			const SQChar *name=0; 
			SQInteger seq=0;
			while((name = sq_getlocal(v,level,seq)))
			{
				SQInteger i;
				SQFloat f;
				const SQChar *s;

				fIndent( level, out );
				out << "Local Variable: " << name;

				seq++;
				switch(sq_gettype(v,-1))
				{
				case OT_NULL:								out << " (NULL)";							break;
				case OT_INTEGER:	sq_getinteger(v,-1,&i); out << " " << i << " (integer)";			break;
				case OT_FLOAT:		sq_getfloat(v,-1,&f);	out << " " << f << " (float)";				break;
				case OT_USERPOINTER:						out << " (USERPOINTER)";					break;
				case OT_STRING:		sq_getstring(v,-1,&s);	out << " \"" << s << "\"";					break;
				case OT_TABLE:								out << " (TABLE)";							break;
				case OT_ARRAY:								out << " (ARRAY)";							break;
				case OT_CLOSURE:							out << " (CLOSURE)";						break;
				case OT_NATIVECLOSURE:						out << " (NATIVECLOSURE)";					break;
				case OT_GENERATOR:							out << " (GENERATOR)";						break;
				case OT_USERDATA:							out << " (USERDATA)";						break;
				case OT_THREAD:								out << " (THREAD)";							break;
				case OT_CLASS:								out << " (CLASS)";							break;
				case OT_INSTANCE:							out << " (INSTANCE)";						break;
				case OT_WEAKREF:							out << " (WEAKREF)";						break;
				case OT_BOOL:		sq_getinteger(v,-1,&i); out << " " << (bool)(i!=0) << " (bool)";	break;
				default: sigassert(0); break;
				}
				sq_pop(v,1);

				out << std::endl;
			}
		}

		void fSqPrintCallstack( HSQUIRRELVM v, std::stringstream& out )
		{
			out << std::endl;
			SQStackInfos si;
			for( u32 level = 1; SQ_SUCCEEDED(sq_stackinfos(v,level,&si)); ++level )
			{
				const SQChar *fn=_SC("unknown");
				const SQChar *src=_SC("unknown");
				if(si.funcname)
					fn = si.funcname;
				if(si.source)
					src = si.source;

				fIndent( level, out );
				out << "Function: " << fn << ", File: " << src << ", Line: " << si.line << std::endl;
				fSqPrintVariables( v, level, out );
				out << std::endl;
			}
		}
		void fSqPrint( HSQUIRRELVM v, const SQChar *s, ... ) 
		{
			va_list arglist;
			va_start(arglist, s);

			char dest[1024];
			vsnprintf(dest, sizeof(dest)/sizeof(dest[0]) - 1, s, arglist);

			va_end( arglist );

			log_line( Log::cFlagScript, dest );
		}
		void fSqCompilerErrorInternal( HSQUIRRELVM v, const SQChar *sErr, const SQChar *sSource, SQInteger line, SQInteger column )
		{
			gCompileErrorLineNo		= line;
			gCompileErrorColumnNo	= column;
			log_warning( Log::cFlagScript, 
				"Compile error in file [" << sSource << "]: [" << sErr << "], line = (" << line << "), character = (" << column << ")" );
		}
		void fSqCompilerError( const char* str, u32 lineNo, u32 columnNo )
		{
			char badLine[1024]={0};

			const char* badLineStart = str;
			for( u32 i = 1; badLineStart && i < lineNo; ++i )
			{
				badLineStart = strchr( badLineStart, '\n' );
				if( badLineStart ) ++badLineStart;
			}

			while( isspace( *badLineStart ) ) ++badLineStart;

			const char* badLineEnd = badLineStart;
			while( *badLineEnd != '\n' && *badLineEnd != '\0' ) ++badLineEnd;

			strncpy( badLine, badLineStart, badLineEnd - badLineStart - 1 );
			log_warning( Log::cFlagScript, "\tline in question: '" << badLine << "'" );
		}
		SQInteger fSqErrorHandler( HSQUIRRELVM v )
		{
			const SQChar *sErr = 0;
			if( sq_gettop( v ) >= 1)
			{
				std::stringstream out;
				if(SQ_SUCCEEDED(sq_getstring(v,2,&sErr)))
				{
					out << sErr << " ===> Printing callstack:";
				}
				else
				{
					out << "[unknown]" << " ===> Printing callstack:";
				}
				fSqPrintCallstack( v, out );
				log_warning( Log::cFlagScript, out.str() );
			}

			return 0;
		}
		static bool fIsNull( byte* pointer )
		{
			return pointer == 0;
		}
	}


	tScriptVm::tScriptVm( )
		: mSq( 0 ), mEmptyTable( 0 ), mRootTable( 0 ), mConstTable( 0 )
	{
		mTimeStamps.fSetCapacity( 128 );
		fNew( );
	}
	tScriptVm::~tScriptVm( )
	{
		fClear( );
	}
	void tScriptVm::fNew( )
	{
		sigassert( !mSq );
		mSq = sq_open( 1024 );
		sigassert( mSq );

		Sqrat::DefaultVM::Set(mSq);

		sq_setprintfunc( mSq, fSqPrint );
		//sq_pushroottable( mSq );
		sq_setcompilererrorhandler( mSq, fSqCompilerErrorInternal );
		sq_newclosure( mSq, fSqErrorHandler, 0 );
		sq_seterrorhandler( mSq );

		sigassert( !mRootTable );
		mRootTable = NEW Sqrat::RootTable( mSq );

		sigassert( !mConstTable );
		mConstTable = NEW Sqrat::ConstTable( mSq );

		sigassert( !mEmptyTable );
		mEmptyTable = NEW Sqrat::Table( mSq );

		fRootTable( ).Func( _SC("is_null"), fIsNull);
	}
	void tScriptVm::fClear( )
	{
		for( u32 i = 0; i < mNamespaces.fCount( ); ++i )
			delete mNamespaces[ i ].mTable;
		mNamespaces.fSetCount( 0 );

		if( mEmptyTable )
		{
			delete mEmptyTable;
			mEmptyTable = 0;
		}
		if( mConstTable )
		{
			delete mConstTable;
			mConstTable = 0;
		}
		if( mRootTable )
		{
			delete mRootTable;
			mRootTable = 0;
		}
		if( mSq )
		{
			HSQUIRRELVM sq = mSq;
			mSq = 0;
			//sq_pop( mSq, 1 );
			sq_close( sq ); 
			sq = 0;
		}
		Sqrat::DefaultVM::Set(0);
	}
	Sqrat::Table& tScriptVm::fNamespace( const char* name )
	{
		tScriptNamespace* find = mNamespaces.fFind( name );
		if( find )
			return *find->mTable;
		Sqrat::Table* table = NEW Sqrat::Table( fSq( ) );
		fRootTable( ).Bind( name, *table );
		mNamespaces.fPushBack( tScriptNamespace( name, table ) );
		return *table;
	}
	void tScriptVm::fReset( )
	{
		fClear( );
		fNew( );
	}
	u32 tScriptVm::fQueryStackTop( ) const
	{
		return sq_gettop( mSq );
	}
	void tScriptVm::fLogStackTop( const char* context, u32 minValueAtWhichToLog ) const
	{
		profile_pix("tScriptVm::fLogStackTop");
		const u32 st = fQueryStackTop( );
		if( st < minValueAtWhichToLog )
			return;
		if( context )
		{
			log_warning( Log::cFlagScript, "sq stack top (" << context << "): " << st );
		}
		else
		{
			log_warning( Log::cFlagScript, "sq stack top: " << st );
		}
	}
	b32 tScriptVm::fGarbageCollect( )
	{
		return SQ_SUCCEEDED( sq_collectgarbage( mSq ) );
	}
	b32 tScriptVm::fCompileString( const char* str, const char* srcFileName )
	{
		if( SQ_FAILED( sq_compilebuffer( mSq, str, ( int )strlen( str ), srcFileName, SQTrue ) ) )
		{
			fSqCompilerError( str, gCompileErrorLineNo, gCompileErrorColumnNo );
			return false;
		}

		return true;
	}
	b32 tScriptVm::fCompileStringAndRun( const char* str, const char* srcFileName )
	{
		if( !fCompileString( str, srcFileName ) )
			return false;
		return fRunCurrent( );
	}
	b32 tScriptVm::fRunCurrent( )
	{
		sq_pushroottable( mSq );
		const b32 success = SQ_SUCCEEDED( sq_call( mSq, 1, false, SQTrue ) );
		sq_pop( mSq, 1 ); //removes the closure
		return success;
	}

	namespace
	{
		SQInteger fWriteClosure( SQUserPointer dstPtr, SQUserPointer srcPtr, SQInteger bytes )
		{
			sigassert( dstPtr && srcPtr );

			Sig::byte* src = ( Sig::byte* )srcPtr;
			tGrowableArray<Sig::byte>& dst = *( tGrowableArray<Sig::byte>* )dstPtr;

			for( s32 i = 0; i < bytes; ++i )
				dst.fPushBack( src[ i ] );

			return bytes;
		}
		SQInteger fReadClosure( SQUserPointer srcPtr, SQUserPointer dstPtr, SQInteger bytes )
		{
			sigassert( dstPtr && srcPtr );

			const Sig::byte** src = ( const Sig::byte** )srcPtr;
			Sig::byte* dst = ( Sig::byte* )dstPtr;

			fMemCpy( dst, *src, bytes );
			*src += bytes;

			return bytes;
		}
	}

	b32 tScriptVm::fGenerateByteCode( tDynamicBuffer& byteCode, b32 endianSwap )
	{
		tGrowableArray<Sig::byte> dst;

		const SQBool wasEndianSwap = sq_getendianswapenabled( );
		sq_endianswapenable( endianSwap );
		const b32 failed = ( SQ_FAILED( sq_writeclosure( mSq, fWriteClosure, &dst ) ) );
		sq_endianswapenable( wasEndianSwap );

		if( failed )
			return false;

		byteCode.fNewArray( dst.fCount( ) );
		fMemCpy( byteCode.fBegin( ), dst.fBegin( ), byteCode.fCount( ) );
		return true;
	}
	b32 tScriptVm::fRegisterScriptFile( tScriptFile& scriptFile, const tResource& res )
	{
		u64* timeStamp = mTimeStamps.fFind( res.fGetPath( ) );
		if( timeStamp && *timeStamp == res.fFileTimeStamp( ) )
			return true; // already compiled

		if( scriptFile.mFlags & tScriptFile::cFlagCompiledByteCode )
		{
			const Sig::byte* src = scriptFile.mByteCode.fBegin( );
			if( SQ_FAILED( sq_readclosure( mSq, fReadClosure, ( SQUserPointer )&src ) ) )
				return false;
		}
		else
		{
			if( !fCompileString( ( const char* )scriptFile.mByteCode.fBegin( ), res.fGetPath( ).fCStr( ) ) )
				return false; // couldn't compile
		}

		if( !fRunCurrent( ) )
			return false;

		if( timeStamp )
			*timeStamp = res.fFileTimeStamp( );
		else
			mTimeStamps.fInsert( res.fGetPath( ), res.fFileTimeStamp( ) );

		return true;
	}

	void tScriptVm::fDumpCallstack( )
	{
		if( Sqrat::DefaultVM::Get( ) )
		{
			log_warning( Log::cFlagScript, "[[[ DumpCallstack ]]]" );
			fSqErrorHandler( Sqrat::DefaultVM::Get( ) );
		}
	}

}

#include "Scripts/tScript64.hpp"
#include "tGameAppBase.hpp"
#include "tAnimPackFile.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tRotateAnimTrack.hpp"
#include "tOrientAnimTrack.hpp"
#include "tOrientBasisAnimTrack.hpp"
#include "tIKAnimTrackBase.hpp"
#include "tDataTableFile.hpp"
#include "tShapeEntity.hpp"
#include "tPathEntity.hpp"
#include "tStateableEntity.hpp"
#include "tProximity.hpp"
#include "tUberBreakableLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"
#include "Logic/tPhysical.hpp"
#include "AI/tScriptableGoal.hpp"
#include "AI/tMotionGoal.hpp"
#include "AI/tSigAIGoal.hpp"
#include "Physics/tCharacterPhysics.hpp"
#include "Physics/tHinge.hpp"
#include "Gfx/tRenderContext.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tFollowPathCamera.hpp"
#include "Gfx/tTextureReference.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gui/tColoredQuad.hpp"
#include "Gui/tTexturedQuad.hpp"
#include "tAnimatedQuad.hpp"
#include "Gui/tMutableTexturedQuad.hpp"
#include "Gui/tText.hpp"
#include "Gui/tLoadScreen.hpp"
#include "Gui/tSaveUI.hpp"
#include "Gui/tWorldSpaceScriptedControl.hpp"
#include "Gui/tLineList.hpp"
#include "Gui/tAsyncTexturedQuad.hpp"
#include "Gui/tGamerPictureQuad.hpp"
#include "Gui/tMovieQuad.hpp"
#include "Gui/tWorldSpaceText.hpp"
#include "tFxFileRefEntity.hpp"
#include "Input/tGamepad.hpp"
#include "IK/tIK.hpp"
#include "Net/tRemoteConnection.hpp"
#include "tLeaderboard.hpp"
#include "Audio/tAudioLogic.hpp"
#include "tTileEntity.hpp"
#include "tGameEffects.hpp"
#include "tAchievements.hpp"

namespace Sig
{
	void tScriptVm::fExportGameFlags(
			const tStringPtr      gameFlagNames[],
			const u32             gameFlagValues[],
			const tStringPtr      gameEnumTypeNames[],
			const u32             gameEnumTypeKeys[],
			const tStringPtr*const gameEnumValueNames[],
			const u32*const       gameEnumValues[]  )
	{
		for( ; !gameFlagNames->fNull( ); ++gameFlagNames, ++gameFlagValues )
			fConstTable( ).Const( gameFlagNames->fCStr( ), ( int )*gameFlagValues );

		for( ; !gameEnumTypeNames->fNull( ); ++gameEnumTypeNames, ++gameEnumTypeKeys )
			fConstTable( ).Const( gameEnumTypeNames->fCStr( ), ( int )*gameEnumTypeKeys );

		for( ; *gameEnumValueNames; ++gameEnumValueNames, ++gameEnumValues )
		{
			const tStringPtr* valueNames = *gameEnumValueNames;
			const u32* values = *gameEnumValues;
			for( ; !valueNames->fNull( ); ++valueNames, ++values )
				fConstTable( ).Const( valueNames->fCStr( ), ( int )*values );
		}
	}
	void tScriptVm::fExportCommonScriptInterfaces( )
	{
		// script types
		fRootTable( ).Func( _SC("DumpCallstack"), &tScriptVm::fDumpCallstack );
		tScript64::fExportScriptInterface( *this );

		// nuts and bolts
		Log::fExportScriptInterface( *this );
		Math::fExportScriptInterface( *this );
		tRandom::fExportScriptInterface( *this );
		tLocalizedString::fExportScriptInterface( *this );
		StringUtil::fExportScriptInterface( *this );

		// application
		tApplication::fExportScriptInterface( *this );
		tGameAppBase::fExportScriptInterface( *this );
		tUser::fExportScriptInterface( *this );

		// logic + entity
		tLogic::fExportScriptInterface( *this );
		Logic::tEvent::fExportScriptInterface( *this );
		tEntity::fExportScriptInterface( *this );
		tStateableEntity::fExportScriptInterface( *this );
		tMeshEntity::fExportScriptInterface( *this );
		tSceneGraph::fExportScriptInterface( *this );
		tShapeEntity::fExportScriptInterface( *this );
		tPathEntity::fExportScriptInterface( *this );
		tSceneRefEntity::fExportScriptInterface( *this );
		tProximity::fExportScriptInterface( *this );
		tUberBreakableLogic::fExportScriptInterface( *this );
		tTileEntity::fExportScriptInterface( *this );

		// animation
		tKeyFrameAnimation::fExportScriptInterface( *this );
		tAnimPackFile::fExportScriptInterface( *this );
		tAnimatedSkeleton::fExportScriptInterface( *this );
		tKeyFrameAnimDesc::fExportScriptInterface( *this );
		tRotateAnimDesc::fExportScriptInterface( *this );
		tOrientAnimDesc::fExportScriptInterface( *this );
		tOrientBasisAnimDesc::fExportScriptInterface( *this );
		IK::fExportScriptInterface( *this );
		tIKAnimTrackDesc::fExportScriptInterface( *this );
		Logic::tAnimatable::fExportScriptInterface( *this );
		tMotionMap::fExportScriptInterface( *this );

		// data table
		tDataTable::fExportScriptInterface( *this );
		tDataTableFile::fExportScriptInterface( *this );

		// goals
		Logic::tGoalDriven::fExportScriptInterface( *this );
		AI::tGoal::fExportScriptInterface( *this );
		AI::tCompositeGoal::fExportScriptInterface( *this );
		AI::tSigAIGoal::fExportScriptInterface( *this );
		AI::tScriptableGoal::fExportScriptInterface( *this );
		AI::tMotionGoal::fExportScriptInterface( *this );
		AI::tTimedMotionGoal::fExportScriptInterface( *this );
		AI::tOneShotMotionGoal::fExportScriptInterface( *this );

		// physics
		Logic::tPhysical::fExportScriptInterface( *this );
		Physics::tStandardPhysics::fExportScriptInterface( *this );
		Physics::tCharacterPhysics::fExportScriptInterface( *this );
		Physics::tOneWayHinge::fExportScriptInterface( *this );

		// graphics
		Gfx::fExportScriptInterface( *this );
		Gfx::tRenderableEntity::fExportScriptInterface( *this );
		Gfx::tFollowPathCameraPointLogic::fExportScriptInterface( *this );
		Gfx::tTextureReference::fExportScriptInterface( *this );
		Gfx::tLightEntity::fExportScriptInterface( *this );

		// gui
		Gui::tCanvas::fExportScriptInterface( *this );
		Gui::tCanvasFrame::fExportScriptInterface( *this );
		Gui::tRenderableCanvas::fExportScriptInterface( *this );
		Gui::tColoredQuad::fExportScriptInterface( *this );
		Gui::tTexturedQuad::fExportScriptInterface( *this );
		Gui::tAnimatedQuad::fExportScriptInterface( *this );
		Gui::tMutableTexturedQuad::fExportScriptInterface( *this );
		Gui::tAsyncTexturedQuad::fExportScriptInterface( *this );
		Gui::tGamerPictureQuad::fExportScriptInterface( *this );
		Gui::tMovieQuad::fExportScriptInterface( *this );
		Gui::tWorldSpaceText::fExportScriptInterface( *this );
		Gui::tText::fExportScriptInterface( *this );
		Gui::tLoadScreen::fExportScriptInterface( *this );
		Gui::tSaveUI::fExportScriptInterface( *this );
		Gui::tWorldSpaceScriptedControl::fExportScriptInterface( *this );
		Gui::tLineList::fExportScriptInterface( *this );

		// input
		Input::tGamepad::fExportScriptInterface( *this );
		Input::tKeyboard::fExportScriptInterface( *this );
		Input::tMouse::fExportScriptInterface( *this );

		// net
		Net::tRemoteConnection::fExportScriptInterface( *this );

		// leaderboard
		tLeaderboard::fExportScriptInterface( *this );

		// Achievements
		tAchievementData::fExportScriptInterface( *this );

		// audio comment
		tAudioLogic::fExportScriptInterface( *this );

		// effects system
		FX::tFxFileRefEntity::fExportScriptInterface( *this );
		tGameEffects::fExportScriptInterface( *this );
	}

}
