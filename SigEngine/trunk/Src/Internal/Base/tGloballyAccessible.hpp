#ifndef __tGloballyAccessible__
#define __tGloballyAccessible__

namespace Sig
{
	template<class tDerived, int cInitListCapacity>
	class tGloballyAccessible
	{
	protected:
		class tGlobalList : public tGrowableArray<tDerived*>
		{
			declare_singleton_define_own_ctor_dtor( tGlobalList );
		public:
			tGlobalList( ) { tGrowableArray<tDerived*>::fSetCapacity( cInitListCapacity ); }
			~tGlobalList( ) { }
			void fAddObject( tDerived* object ) { tGrowableArray<tDerived*>::fPushBack( object ); }
			void fRemoveObject( tDerived* object ) { tGrowableArray<tDerived*>::fFindAndErase( object ); }
		};
		void fAddToGlobalList( ) { tGlobalList::fInstance( ).fAddObject( static_cast<tDerived*>( this ) ); }
		void fRemoveFromGlobalList( ) { tGlobalList::fInstance( ).fRemoveObject( static_cast<tDerived*>( this ) ); }
	public:
		tGloballyAccessible( ) { fAddToGlobalList( ); }
		~tGloballyAccessible( ) { fRemoveFromGlobalList( ); }
		static const tGrowableArray<tDerived*>& fGlobalList( ) { return tGlobalList::fInstance( ); }
	};
}

#endif//__tGloballyAccessible__

