#ifndef __tStateableEntity__
#define __tStateableEntity__

namespace Sig
{

	class base_export tStateableEntity : public tEntity
	{
		define_dynamic_cast( tStateableEntity, tEntity );
	private:
		u16 mStateMask;
		u16 pad0;

	public:
		static const u32 cMaxStateMaskValue = 0xFFFF;

		tStateableEntity( u16 stateMask = cMaxStateMaskValue );

		u16				fStateMask( ) const				 { return mStateMask; }
		void			fSetStateMask( u16 mask )		 { mStateMask = mask; }

		b32				fStateEnabled( u32 index ) const { return fTestBits( mStateMask, (1 << index) ); }

		// Called by the game app when an object has changed states. Derived entity types can make use of this
		//  how ever they want. If this base function gets called, it will call a similar method on fLogic( ).
		virtual void	fStateMaskEnable( u32 index );	 
		
		static void		fSetStateMaskRecursive( tEntity& root, u16 mask )
		{
			tStateableEntity* stateable = root.fDynamicCast< tStateableEntity >( );
			if( stateable ) stateable->mStateMask = mask;
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				fSetStateMaskRecursive( *root.fChild( i ), mask );
		}

		static void		fStateMaskEnableRecursive( tEntity& root, u16 index )
		{
			tStateableEntity* stateable = root.fDynamicCast< tStateableEntity >( );
			if( stateable ) stateable->fStateMaskEnable( index );
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				fStateMaskEnableRecursive( *root.fChild( i ), index );
		}

		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tStateableEntity );

}

#endif//__tStateableEntity__
