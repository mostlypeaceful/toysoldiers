#ifndef __RttiClassHierarchy__
#define __RttiClassHierarchy__

namespace Sig { namespace Rtti
{
	///
	/// Useful for storing unique instances of fully derived types, all of which derive
	/// from a common base type; i.e., there will only ever be one BderivesFromA, and
	/// only one CderivesFromA, ever in this map; you can then look up these instances
	/// by tClassId, as well as iterate over them.
	template<class t>
	class tClassHierarchyMap : public tHashTable<tClassId,t*>
	{
		declare_singleton_define_own_ctor_dtor( tClassHierarchyMap );

		inline tClassHierarchyMap( ) : tHashTable<tClassId,t*>( 32 ) { }
		inline ~tClassHierarchyMap( ) { }
	};

	///
	/// \brief Facilitates creating a new class hierarchy which can leverage the tClassHierarchyMap.
	///
	/// By deriving from this class and passing in your derived type as the template argument,
	/// you can make use of the tManager object for accessing any classes that in turn derive 
	/// from your class ('t').
	template<class t>
	class tClassHierarchyObject
	{
	private:
		
		const tClassId mCid;

	public:

		typedef tClassHierarchyMap<t> tManager;

		tClassHierarchyObject( tClassId cid ) : mCid( cid )
		{
			t** find = tManager::fInstance( ).fFind( mCid );
			if( !find )
				tManager::fInstance( ).fInsert( mCid, static_cast<t*>( this ) );
		}

		virtual ~tClassHierarchyObject( )
		{
			tManager::fInstance( ).fRemove( mCid );
		}
	};


}}

#endif//__RttiClassHierarchy__
