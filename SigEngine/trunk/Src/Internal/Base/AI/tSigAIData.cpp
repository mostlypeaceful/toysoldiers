#include "BasePch.hpp"
#include "tSigAIData.hpp"
#include "tLogicEvent.hpp"

namespace Sig { namespace AI
{

	devvar( b32, AI_Data_Log, false );

	namespace
	{
		void fSetValueScript( tAIData* data, u32 index, b32 value )
		{
			data->fSetValue( index, value );
		}

		//void fSetValueForceScript( tAIData* data, u32 index, b32 value )
		//{
		//	data->fSetValue( index, value, true );
		//}
	}


	b32 tAIData::fValue( u32 index ) const 
	{ 
		if( index >= mItems.fCount( ) )
		{
			log_warning( "Cannot get value for ai data index " << index << " - likely need a new build" );
			return false;
		}

		b32 value = mItems[ index ].fValue( );

		if( AI_Data_Log )
			log_line( 0, "Rquested: " << index << " Value: " << value );

		return value;
	}


	void tAIData::fSetValue( u32 index, b32 value, Logic::tEventContext* context )
	{
		if( index >= mItems.fCount( ) )
		{
			log_warning( "Cannot set value for ai data index " << index << " - likely need a new build" );
			return;
		}

		tAIDataItem& item = mItems[ index ];
		b32 changed = item.fValue( ) != value;

		item.fSetValue( value );

		if( changed && item.fFireEventOnChange( ) )
		{
			sigassert( mEventHandler );
			if( mEventHandler->fOwnerEntity( ) )
				mEventHandler->fOwnerEntity( )->fHandleLogicEvent( Logic::tEvent( fToEvent( index, value ), context ) );
			else
				log_warning( "EventHandler entity not set before logic event was attempted to be sent." );
		}

		if( changed && AI_Data_Log )
			log_line( 0, "Changed: " << index << " Value: " << value );
	}

	void tAIData::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAIData, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("Value"),					&tAIData::fValue)
			.GlobalFunc(_SC("SetValue"),		&fSetValueScript)
			//.StaticFunc(_SC("SetValueForce"),	&fSetValueForceScript)
			;

		vm.fNamespace( _SC("AI") ).Bind(_SC("AIData"), classDesc);
	}
	
}}

