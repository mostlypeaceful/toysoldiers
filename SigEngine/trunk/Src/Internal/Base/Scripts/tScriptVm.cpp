#include "BasePch.hpp"
#include "tScriptVm.hpp"
#include "tScriptFile.hpp"
#include "Memory/tHeap.hpp"
#include "Allocators.hpp"

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
		Memory::tAllocStamp cSQStamp( __FILE__, __LINE__ );

		void* fSqMalloc( u32 size )
		{
			void* mem = Allocators::gScript->fAlloc(size);
			if( !mem )
				Log::fFatalError();
			return mem;
		}
		void fSqFree(void* mem, u32 /*memSize*/)
		{
			Allocators::gScript->fFree(mem);
		}
		void* fSqRealloc(void* oldMem, u32 /*oldSize*/, u32 size)
		{
			void* mem = Allocators::gScript->fRealloc(oldMem,size);
			if( !mem )
				Log::fFatalError();
			return mem;
		}

		define_static_function( fSqSetAllocFuncs )
		{
			sq_setallocfuncs( fSqMalloc, fSqRealloc, fSqFree );
			Sqrat::ErrorHandling::Enable( true );
		}
#endif//target_game

		u32 gCompileErrorLineNo=0;
		u32 gCompileErrorColumnNo=0;

		static void fIndent( std::ostream& os, u32 numTimes )
		{
			for( u32 i = 0; i < numTimes; ++i )
				os << "     ";
		}

		static void fSqPrintVariables( std::ostream& os, HSQUIRRELVM v, u32 level )
		{
			const SQChar *name=0; 
			SQInteger seq=0;
			while((name = sq_getlocal(v,level,seq)))
			{
				SQInteger i;
				SQFloat f;
				const SQChar *s;

				fIndent( os, level );
				os << "Local Variable: " << name;

				seq++;
				switch(sq_gettype(v,-1))
				{
				case OT_NULL:								os << " (NULL)";								break;
				case OT_INTEGER:	sq_getinteger(v,-1,&i); os << " " << i << " (integer)";					break;
				case OT_FLOAT:		sq_getfloat(v,-1,&f);	os << " " << f << " (float)";					break;
				case OT_USERPOINTER:						os << " (USERPOINTER)";							break;
				case OT_STRING:		sq_getstring(v,-1,&s);	os << " \"" << s << "\"";						break;
				case OT_TABLE:								os << " (TABLE)";								break;
				case OT_ARRAY:								os << " (ARRAY)";								break;
				case OT_CLOSURE:							os << " (CLOSURE)";								break;
				case OT_NATIVECLOSURE:						os << " (NATIVECLOSURE)";						break;
				case OT_GENERATOR:							os << " (GENERATOR)";							break;
				case OT_USERDATA:							os << " (USERDATA)";							break;
				case OT_THREAD:								os << " (THREAD)";								break;
				case OT_CLASS:								os << " (CLASS)";								break;
				case OT_INSTANCE:							os << " (INSTANCE)";							break;
				case OT_WEAKREF:							os << " (WEAKREF)";								break;
				case OT_BOOL:		sq_getinteger(v,-1,&i); os << " " << (i?"TRUE":"FALSE") << " (bool)";	break;
				default: sigassert(0); break;
				}
				sq_pop(v,1);

				os << "\n";
			}
		}

		void fSqPrintCallstack( std::ostream& os, HSQUIRRELVM v )
		{
			os << "\n";
			SQStackInfos si;
			for( u32 level = 1; SQ_SUCCEEDED(sq_stackinfos(v,level,&si)); ++level )
			{
				const SQChar *fn=_SC("unknown");
				const SQChar *src=_SC("unknown");
				if(si.funcname)
					fn = si.funcname;
				if(si.source)
					src = si.source;

				fIndent( os, level );
				os << "Function: " << fn << ", File: " << src << ", Line: " << si.line << "\n";
				fSqPrintVariables( os, v, level );
				os << "\n";
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
			log_assert( false,
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
			log_warning( "\tline in question: '" << badLine << "'" );
		}

		void fSqErrorHandlerFormat( std::ostream& os, HSQUIRRELVM v )
		{
			const SQChar *sErr = 0;
			if( sq_gettop( v ) >= 1)
			{
				if(!SQ_SUCCEEDED(sq_getstring(v,2,&sErr)))
					sErr = _SC("[unknown]");

				os << sErr << " ===> Printing callstack:";
				fSqPrintCallstack( os, v );
			}
		}

		SQInteger fSqErrorHandler( HSQUIRRELVM v )
		{
			std::stringstream ss;
			fSqErrorHandlerFormat( ss, v );
			Log::fPrintf( 0, ss.str().c_str() );
			return 0;
		}

		SQInteger fSqErrorHandlerAsserting( HSQUIRRELVM v )
		{
			std::stringstream ss;
			fSqErrorHandlerFormat( ss, v );
			log_assert( 0, ss.str() );
			return 0;
		}

		static bool fIsNull( byte* pointer )
		{
			return pointer == 0;
		}

		static bool fIsEqual( byte* pointerA, byte* pointerB )
		{
			return pointerA == pointerB;
		}
	}


	tScriptVm::tScriptVm( )
		: mSq( NULL )
		, mEmptyTable( NULL )
		, mRootTable( NULL )
		, mConstTable( NULL )
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
		sq_newclosure( mSq, fSqErrorHandlerAsserting, 0 );
		sq_seterrorhandler( mSq );

		sigassert( !mRootTable );
		mRootTable = NEW Sqrat::RootTable( mSq );

		sigassert( !mConstTable );
		mConstTable = NEW Sqrat::ConstTable( mSq );

		sigassert( !mEmptyTable );
		mEmptyTable = NEW Sqrat::Table( mSq );

		fRootTable( ).Func( _SC("is_null"), fIsNull);
		fRootTable( ).Func( _SC("is_equal"), fIsEqual);
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
		const u32 st = fQueryStackTop( );
		if( st < minValueAtWhichToLog )
			return;
		if( context )
		{
			log_warning( "sq stack top (" << context << "): " << st );
		}
		else
		{
			log_warning( "sq stack top: " << st );
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
		if( Sqrat::DefaultVM::Get( ) && sq_gettop( Sqrat::DefaultVM::Get( ) ) >= 1 )
		{
			Log::fPrintf( 0, "!WARNING! >> [[[ DumpCallstack ]]]\n" );
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
#include "Anim/tCharacterMoveAnimTrackFPS.hpp"

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
#include "Physics/tPhysicsWorld.hpp"
#include "Physics/tCharacterPhysics.hpp"
#include "Physics/tHinge.hpp"
#include "Physics/tRagDoll.hpp"
#include "Physics/tRagDollAnimTrack.hpp"
#include "Gfx/tRenderContext.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tFollowPathCamera.hpp"
#include "Gfx/tTextureReference.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gfx/tLight.hpp"
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
#include "Gui/tWorldToScreenSpaceText.hpp"
#include "tFxFileRefEntity.hpp"
#include "Input/tGamepad.hpp"
#include "IK/tIK.hpp"
#include "Net/tRemoteConnection.hpp"
#include "tLeaderboard.hpp"
#include "Audio/tAudioLogic.hpp"
#include "tTileEntity.hpp"
#include "tGameEffects.hpp"
#include "tAchievements.hpp"
#include "Gui/tFuiCanvas.hpp"
#include "Gui/tFuiLoadCanvas.hpp"

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
		Audio::tSystem::fExportScriptInterface( *this );

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
		Anim::tTrackInput::fExportScriptInterface( *this );
		Anim::tAnimTrackDesc::fExportScriptInterface( *this );
		Anim::tAnimatedSkeleton::fExportScriptInterface( *this );
		Anim::tKeyFrameAnimDesc::fExportScriptInterface( *this );
		Anim::tRotateAnimDesc::fExportScriptInterface( *this );
		Anim::tOrientAnimDesc::fExportScriptInterface( *this );
		Anim::tOrientBasisAnimDesc::fExportScriptInterface( *this );
		IK::fExportScriptInterface( *this );
		Anim::tCharacterMoveFPSAnimDesc::fExportScriptInterface( *this );
		Anim::tIKAnimTrackDesc::fExportScriptInterface( *this );
		Logic::tAnimatable::fExportScriptInterface( *this );
		Anim::tMotionMap::fExportScriptInterface( *this );

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
		Physics::tPhysicsWorld::fExportScriptInterface( *this );
		Physics::tStandardPhysics::fExportScriptInterface( *this );
		Physics::tCharacterPhysics::fExportScriptInterface( *this );
		Physics::tOneWayHinge::fExportScriptInterface( *this );
		Physics::tRagDoll::fExportScriptInterface( *this );
		Anim::tRagDollAnimTrackDesc::fExportScriptInterface( *this );

		// graphics
		Gfx::fExportScriptInterface( *this );
		Gfx::tRenderableEntity::fExportScriptInterface( *this );
		Gfx::tFollowPathCameraPointLogic::fExportScriptInterface( *this );
		Gfx::tTextureReference::fExportScriptInterface( *this );
		Gfx::tLightEntity::fExportScriptInterface( *this );
		Gfx::tLight::fExportScriptInterface( *this );

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
		Gui::tWorldToScreenSpaceText::fExportScriptInterface( *this );
		Gui::tText::fExportScriptInterface( *this );
		Gui::tLoadScreen::fExportScriptInterface( *this );
		Gui::tSaveUI::fExportScriptInterface( *this );
		Gui::tWorldSpaceScriptedControl::fExportScriptInterface( *this );
		Gui::tLineList::fExportScriptInterface( *this );
		Gui::tFuiCanvas::fExportScriptInterface( *this );
		Gui::tFuiLoadCanvas::fExportScriptInterface( *this );

		// input
		Input::tGamepad::fExportScriptInterface( *this );

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

#if !defined( game_release )
// Workaround 360's inability to call static member functions directly from the Immediate Window in Visual Studio
void fDumpScriptCallstack( )
{
	Sig::tScriptVm::fDumpCallstack( );
}
#endif // !defined( game_release )
