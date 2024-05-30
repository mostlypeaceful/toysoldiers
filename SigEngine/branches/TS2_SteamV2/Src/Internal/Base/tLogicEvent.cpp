#include "BasePch.hpp"
#include "tLogicEvent.hpp"

namespace Sig
{

	namespace Logic
	{
		void tEvent::fScriptSetup( u32 eventID, const Sqrat::Object& obj )
		{
			mEventId = eventID;
			mContext = tScriptOrCodeObjectPtr<tEventContext>( obj );
		}

		namespace
		{
			static Logic::tStringEventContext* fAsStringEventContext( const Logic::tEventContext* obj )
			{
				return obj ? obj->fDynamicCast< Logic::tStringEventContext >( ) : NULL;
			}

			static Logic::tIntEventContext* fAsIntEventContext( const Logic::tEventContext* obj )
			{
				return obj ? obj->fDynamicCast< Logic::tIntEventContext >( ) : NULL;
			}

			static Logic::tBoolEventContext* fAsBoolEventContext( const Logic::tEventContext* obj )
			{
				return obj ? obj->fDynamicCast< Logic::tBoolEventContext >( ) : NULL;
			}

			static Logic::tObjectEventContext* fAsObjectEventContext( const Logic::tObjectEventContext* obj )
			{
				return obj ? obj->fDynamicCast< Logic::tObjectEventContext >( ) : NULL;
			}
		}

		void tEvent::fExportScriptInterface( tScriptVm& vm )
		{
			Sqrat::Class<Logic::tEvent, Sqrat::DefaultAllocator<Logic::tEvent> > classDescEvent( vm.fSq( ) );
			classDescEvent
				.StaticFunc(_SC("Construct"), &Logic::tEvent::fConstructDefault)
				.Prop(_SC("Id"),		&Logic::tEvent::fEventId)
				.Prop(_SC("Context"),	&Logic::tEvent::fBaseContext)
				.Func(_SC("Setup"),		&Logic::tEvent::fScriptSetup)
				;
			vm.fRootTable( ).Bind( _SC("LogicEvent"), classDescEvent );

			Sqrat::Class<Logic::tEventContext, Sqrat::DefaultAllocator<Logic::tEventContext> > classDescContext( vm.fSq( ) );
			vm.fRootTable( ).Bind( _SC("EventContext"), classDescContext );

			Sqrat::Class<Logic::tStringEventContext, Sqrat::DefaultAllocator<Logic::tStringEventContext> > classDescSContext( vm.fSq( ) );
			classDescSContext
				.StaticFunc(_SC("Construct"), &Logic::tStringEventContext::fConstructDefault)
				.Prop(_SC("String"),	&Logic::tStringEventContext::fString,	&Logic::tStringEventContext::fSetString)
				.StaticFunc(_SC("Convert"), &fAsStringEventContext)
				;
			vm.fRootTable( ).Bind( _SC("StringEventContext"), classDescSContext );

			Sqrat::Class<Logic::tIntEventContext, Sqrat::DefaultAllocator<Logic::tIntEventContext> > classDescIContext( vm.fSq( ) );
			classDescIContext
				.StaticFunc(_SC("Construct"), &Logic::tIntEventContext::fConstructDefault)
				.Prop(_SC("Int"),	&Logic::tIntEventContext::fInt,	&Logic::tIntEventContext::fSetInt)
				.StaticFunc(_SC("Convert"), &fAsIntEventContext)
				;
			vm.fRootTable( ).Bind( _SC("IntEventContext"), classDescIContext );

			Sqrat::Class<Logic::tBoolEventContext, Sqrat::DefaultAllocator<Logic::tBoolEventContext> > classDescBContext( vm.fSq( ) );
			classDescBContext
				.StaticFunc(_SC("Construct"), &Logic::tBoolEventContext::fConstructDefault)
				.Prop(_SC("Bool"),	&Logic::tBoolEventContext::fBool)
				.StaticFunc(_SC("Convert"), &fAsBoolEventContext)
				;
			vm.fRootTable( ).Bind( _SC("BoolEventContext"), classDescBContext );

			Sqrat::Class<Logic::tObjectEventContext, Sqrat::DefaultAllocator<Logic::tObjectEventContext> > classDescOContext( vm.fSq( ) );
			classDescOContext
				.StaticFunc(_SC("Construct"), &Logic::tObjectEventContext::fConstructDefault)
				.Prop(_SC("Object"),	&Logic::tObjectEventContext::fObject,	&Logic::tObjectEventContext::fSetObject)
				.StaticFunc(_SC("Convert"), &fAsObjectEventContext)
				;
			vm.fRootTable( ).Bind( _SC("ObjectEventContext"), classDescOContext );
		}
	}
}



