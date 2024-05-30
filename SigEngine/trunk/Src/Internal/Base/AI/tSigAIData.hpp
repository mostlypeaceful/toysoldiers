#ifndef __tAIData__
#define __tAIData__

namespace Sig { namespace Logic
{
	class tEventContext;
}}

namespace Sig { namespace AI
{

	// This object combines the functionality of flags and events.
	// Entries in this object can be evaluated as a flag (true or false)
	// It also will optionally generate an event for when the flags change.
	// It is abstracted so that designers can declare these flags in the ai tool and continue developing goals based on them.
	// The game programmer can then come along later and implement them.

	/* Sample data 

		enum tDataIndex
		{
			cHasTarget,
			cHasPath,
			cCanAttack,
		}

	*/

	class tAIData
	{
		debug_watch( tAIData );
	public:
		tAIData( ) : mEventHandler( NULL ) { }

		// call this with GameFlags::cAIFLAG_COUNT
		void fInitialize( u32 eventCount ) { mItems.fSetCount( eventCount ); }
		void fSetEventHandler( tLogic* handler ) { mEventHandler = handler; }

		b32 fValue( u32 index ) const;
		void fSetValue( u32 index, b32 value, Logic::tEventContext* context = NULL );
		u32 fValueCount( ) const { return mItems.fCount( ); }

	public:
		static u32 fToEvent( u32 index, b32 value )
		{
			u32 addition = value ? 1 : 0;
			return cEventBase + index * 2 + addition;
		}

		static void fExportScriptInterface( tScriptVm& vm );

#ifdef sig_devmenu
#define fAIDataLogFlags( ss, data, flagToStringFunc ) \
		{ \
			std::string output = "AI Flags Set: "; \
			for( u32 i = 0; i < data.fValueCount( ); ++i ) \
				if( data.fValue( i ) ) \
					output += std::string( "\n  " ) + flagToStringFunc( i ).fCStr( ); \
			ss << output << std::endl; \
		}
#endif

	private:
		struct tAIDataItem
		{
			debug_watch( tAIDataItem );

			b8 mData;

			enum tDataIndex
			{
				cValue		= (1<<0),
				cFireEvent	= (1<<1) //Set this true if you want an event fired when the value changes.
			};

			// fires an event by default
			tAIDataItem( ) 
				: mData( cFireEvent ) 
			{ }

			b32 fValue( ) const { return fTestBits( mData, cValue ); }
			void fSetValue( b32 value ) { mData = fSetClearBits( mData, cValue, value ); }

			b32 fFireEventOnChange( ) const { return fTestBits( mData, cFireEvent ); }
			void fSetFireEventOnChange( b32 fire ) { mData = fSetClearBits( mData, cFireEvent, fire ); }
		};

		tLogic* mEventHandler;
		tGrowableArray< tAIDataItem > mItems;

		static const u32 cEventBase = 0xFFFF; //this should keep us clear of the other game events.
	};
	
}}

#endif//__tAIData__
