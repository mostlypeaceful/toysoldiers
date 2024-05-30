#ifndef __tLogicEvent__
#define __tLogicEvent__

namespace Sig { namespace Logic
{

	class tEventContext : public Rtti::tSerializableBaseClass, public tRefCounterCopyable
	{
		debug_watch( tEventContext );
		define_dynamic_cast_base( tEventContext );
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tEventContext, 0x93FF1B7D );
	public:
		virtual ~tEventContext( ) { }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			fSaveLoadDerived( archive );
		}

	private:
		virtual void fSaveLoadDerived( tGameArchive& archive ) { log_warning_unimplemented( ); }
	};
	typedef tRefCounterPtr<tEventContext> tEventContextPtr;


	class tStringEventContext : public Logic::tEventContext
	{
		debug_watch( tStringEventContext );
		define_dynamic_cast( tStringEventContext, Logic::tEventContext );
		implement_rtti_serializable_base_class( tStringEventContext, 0x536C745E );
	public:
		tStringPtr mString;

	public:
		static tStringEventContext fConstructDefault( const tStringPtr& str = tStringPtr( ) ) { return tStringEventContext( str ); }

		tStringEventContext( const tStringPtr& str = tStringPtr( ) ) : mString( str ) { }

		const tStringPtr& fString( ) const { return mString; }
		void fSetString( const tStringPtr& str ) { mString = str; }

	private:
		virtual void fSaveLoadDerived( tGameArchive& archive );
	};

	class tIntEventContext : public Logic::tEventContext
	{
		debug_watch( tIntEventContext );
		define_dynamic_cast( tIntEventContext, Logic::tEventContext );
		implement_rtti_serializable_base_class( tIntEventContext, 0x99A7774D );
	public:
		u32 mInt;

	public:
		static tIntEventContext fConstructDefault( u32 i = 0 ) { return tIntEventContext( i ); }

		tIntEventContext( u32 i = 0 ) : mInt( i ) { }

		int fInt( ) const { return mInt; }
		void fSetInt( int i ) { mInt = i; }

	private:
		virtual void fSaveLoadDerived( tGameArchive& archive );
	};

	class tBoolEventContext : public Logic::tEventContext
	{
		debug_watch( tBoolEventContext );
		define_dynamic_cast( tBoolEventContext, Logic::tEventContext );
		implement_rtti_serializable_base_class( tBoolEventContext, 0x60303F9B );
	public:

		bool mBool;

	public:
		static tBoolEventContext fConstructDefault( bool b = false ) { return tBoolEventContext( b ); }

		tBoolEventContext( bool b = false ) : mBool( b ) { }

		bool fBool( ) const { return mBool; }

	private:
		virtual void fSaveLoadDerived( tGameArchive& archive );
	};

	class tObjectEventContext : public Logic::tEventContext
	{
		debug_watch( tObjectEventContext );
		define_dynamic_cast( tObjectEventContext, Logic::tEventContext );
	public:
		Sqrat::Object mObject;

	public:
		static tObjectEventContext fConstructDefault( const Sqrat::Object& obj = Sqrat::Object( ) ) { return tObjectEventContext( obj ); }

		tObjectEventContext( const Sqrat::Object& obj = Sqrat::Object( ) ) : mObject( obj ) { }

		const Sqrat::Object& fObject( ) const { return mObject; }
		void fSetObject( const Sqrat::Object& obj ) { mObject = obj; }
	};

	class tVec3fEventContext : public Logic::tEventContext
	{
		debug_watch( tVec3fEventContext );
		define_dynamic_cast( tVec3fEventContext, Logic::tEventContext );

	public:
		Math::tVec3f mVec3f;

	public:
		static tVec3fEventContext fConstructDefault( Math::tVec3f vec3f ) { return tVec3fEventContext( vec3f ); }

		tVec3fEventContext( Math::tVec3f vec3f = Math::tVec3f::cZeroVector )
			: mVec3f( vec3f )
		{
		}

		Math::tVec3f fVec3f( ) const { return mVec3f; }
	};

	class tEvent
	{
		debug_watch( tEvent );
		u32 mEventId;
		tScriptOrCodeObjectPtr<tEventContext> mContext;
	public:
		static tEvent fConstructDefault( u32 eid ) { return tEvent( eid ); }
	public:
		tEvent( ) : mEventId( ApplicationEvent::cNullEventFlag ) { }
		explicit tEvent( u32 eid ) : mEventId( eid ) { }
		tEvent( u32 eid, tEventContext* context ) : mEventId( eid ), mContext( context ) { }
		tEvent( u32 eid, const tEventContextPtr& context ) : mEventId( eid ), mContext( context.fGetRawPtr( ) ) { }
		u32 fEventId( ) const { return mEventId; }

		b32 fHasContext( ) const { return !mContext.fIsNull( ); }
		tEventContext* fBaseContext( ) const { return mContext.fCodeObject( ); }

		template<class tDerivedEventContext>
		tDerivedEventContext* fContext( ) const { return fBaseContext( ) ? fBaseContext( )->fDynamicCast< tDerivedEventContext >( ) : NULL; }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mEventId );

			tEventContextPtr codeObjectPtr( mContext.fCodeObject( ) );
			if( archive.fMode( ) == cGameArchiveModeSave )
			{
				archive.fSaveLoad( codeObjectPtr );
			}
			else
			{
				archive.fSaveLoad( codeObjectPtr );
				mContext = tScriptOrCodeObjectPtr<tEventContext>( codeObjectPtr.fGetRawPtr( ) );
			}
		}

		void fScriptSetup( u32 eventID, const Sqrat::Object& obj );

		static void fExportScriptInterface( tScriptVm& vm );
	};
}}

#endif//__tLogicEvent__
