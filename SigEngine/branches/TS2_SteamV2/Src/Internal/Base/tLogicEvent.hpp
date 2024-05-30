#ifndef __tLogicEvent__
#define __tLogicEvent__
#include "Scripts/tScriptVm.hpp"
#include "tSync.hpp"

namespace Sig { namespace Logic
{

	class tEventContext : public tRefCounter
	{
		define_dynamic_cast_base( tEventContext );
	public:
		virtual ~tEventContext( ) { }
	};
	typedef tRefCounterPtr<tEventContext> tEventContextPtr;


	class tStringEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tStringEventContext, Logic::tEventContext );
		define_class_pool_new_delete( tStringEventContext, 16 )
	public:
		tStringPtr mString;

	public:
		static tStringEventContext fConstructDefault( const tStringPtr& str = tStringPtr( ) ) { return tStringEventContext( str ); }

		tStringEventContext( const tStringPtr& str = tStringPtr( ) ) : mString( str ) { }

		const tStringPtr& fString( ) const { return mString; }
		void fSetString( const tStringPtr& str ) { mString = str; }
	};

	class tIntEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tIntEventContext, Logic::tEventContext );
		define_class_pool_new_delete( tIntEventContext, 16 )
	public:
		u32 mInt;

	public:
		static tIntEventContext fConstructDefault( u32 i = 0 ) { return tIntEventContext( i ); }

		tIntEventContext( u32 i = 0 ) : mInt( i ) { }

		int fInt( ) const { return mInt; }
		void fSetInt( int i ) { mInt = i; }
	};

	class tBoolEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tBoolEventContext, Logic::tEventContext );
		define_class_pool_new_delete( tBoolEventContext, 16 )
	public:

		bool mBool;

	public:
		static tBoolEventContext fConstructDefault( bool b = false ) { return tBoolEventContext( b ); }

		tBoolEventContext( bool b = false ) : mBool( b ) { }

		bool fBool( ) const { return mBool; }
	};

	class tObjectEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tObjectEventContext, Logic::tEventContext );
		define_class_pool_new_delete( tObjectEventContext, 16 )
	public:
		Sqrat::Object mObject;

	public:
		static tObjectEventContext fConstructDefault( const Sqrat::Object& obj = Sqrat::Object( ) ) { return tObjectEventContext( obj ); }

		tObjectEventContext( const Sqrat::Object& obj = Sqrat::Object( ) ) : mObject( obj ) { }

		const Sqrat::Object& fObject( ) const { return mObject; }
		void fSetObject( const Sqrat::Object& obj ) { mObject = obj; }
	};

	class tEvent
	{
		u32 mEventId;
		tScriptOrCodeObjectPtr<tEventContext> mContext;
	public:
		static tEvent fConstructDefault( u32 eid ) { return tEvent( eid ); }
	public:
		static const u32 cNull = 0;
		static const u32 cApplicationEventFlag		= ( 1 << 16 ); // bits 0-15 reserved for event IDs
		static const u32 cGameEventFlag				= ( 1 << 17 );
		static const u32 cPhysicalEventFlag			= ( 1 << 18 );
		static const u32 cAnimationEventFlag		= ( 1 << 19 );
	public:
		inline tEvent( ) : mEventId( cNull ) { }
		inline explicit tEvent( u32 eid ) : mEventId( eid ) { }
		inline tEvent( u32 eid, tEventContext* context ) : mEventId( eid ), mContext( context ) { }
		inline tEvent( u32 eid, const tEventContextPtr& context ) : mEventId( eid ), mContext( context.fGetRawPtr( ) ) { }
		inline u32 fEventId( ) const { return mEventId; }

		inline b32 fHasContext( ) const { return !mContext.fIsNull( ); }
		inline tEventContext* fBaseContext( ) const { return mContext.fCodeObject( ); }

		template<class tDerivedEventContext>
		inline tDerivedEventContext* fContext( ) const { return fBaseContext( ) ? fBaseContext( )->fDynamicCast< tDerivedEventContext >( ) : NULL; }

		void fScriptSetup( u32 eventID, const Sqrat::Object& obj );

		static void fExportScriptInterface( tScriptVm& vm );
	};
}}

#endif//__tLogicEvent__
