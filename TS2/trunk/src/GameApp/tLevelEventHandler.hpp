#ifndef __tLevelEventHandler__
#define __tLevelEventHandler__

namespace Sig
{
	class tUnitLogic;

	class tLevelEventHandler : public tRefCounter
	{
	public:
		struct tLevelEvent : public tGrowableArray<Sqrat::Function>
		{
			void fFire( tUnitLogic* unitLogic );
		};
		
	public:
		tLevelEventHandler( );

		void fAddObserver( GameFlags::tLEVEL_EVENT type, Sqrat::Function func );
		void fFire( GameFlags::tLEVEL_EVENT type, tUnitLogic* unitLogic );
	
	private:
		tFixedArray<tLevelEvent,GameFlags::cLEVEL_EVENT_COUNT> mEvents;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tLevelEventHandler );
}

#endif //__tLevelEventHandler__
