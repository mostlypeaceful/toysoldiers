#ifndef __tEntityData__
#define __tEntityData__

namespace Sig
{
	/*

		tEntityData is the serialization base for supplemental entity information.
		It works like the tEntityDef in that it's a base pointer that can be stored in an array on an entity,
		serialized, and reinstantiated in the game without the engine needing to know about the derived types.

		As long as the application boots with an RTTI factory registered for the serialized information, it will be instantiated.

		This is the framework for the SigEd plugins, allowing the tools to optionally attribute additional binary information to Sigmls and Path entites.
		Such as minimap information, physics, cinematics, quests, etc.
	*/

	class base_export tEntityData : public Rtti::tSerializableBaseClass, public tRefCounter
	{
		declare_reflector( );
		define_dynamic_cast_base( tEntityData );
		implement_rtti_serializable_base_class( tEntityData, 0xAACE494 );
	public:
		tEntityData( ) { }
		tEntityData( tNoOpTag ) { }
		virtual ~tEntityData( ) { }
	};

	class base_export tEntityDataArray : public tUncopyable
	{
		declare_reflector( );
	public:
		tEntityDataArray( ) 
		{ }

		tEntityDataArray( tNoOpTag )
			: mData( cNoOpTag )
		{ }

		~tEntityDataArray( )
		{
			for( u32 i = 0; i < mData.fCount( ); ++i )
				delete mData[ i ];
		}

		template< typename t >
		t* fFirstDataOfType( )
		{
			for( u32 i = 0; i < mData.fCount( ); ++i )
			{
				t* test = mData[ i ]->fDynamicCast< t > ( );
				if( test )
					return test;
			}
			return NULL;
		}

		tDynamicArray< tLoadInPlacePtrWrapper< tEntityData > > mData;
	};

}

#endif//__tEntityData__
