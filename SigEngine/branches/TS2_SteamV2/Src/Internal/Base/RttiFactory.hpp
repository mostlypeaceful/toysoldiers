#ifndef __RttiFactory__
#define __RttiFactory__

namespace Sig { namespace Rtti
{

	///
	/// \brief Create an instance of a class registered using the factory_registrar.
	/// \note This function will sigassert at runtime if the class has not been registered
	/// for factory creation. See tFactoryRegistrar below.
	base_export void* fNewClass( tClassId cid, b32 noopConstruct=false );

	///
	/// \brief Create an instance of a class registered using the factory_registrar, 
	/// using the supplied pre-allocated memory.
	/// \note This function will sigassert at runtime if the class has not been registered
	/// for factory creation. See tFactoryRegistrar below.
	base_export void* fNewClassInPlace( tClassId cid, void* p, b32 noopConstruct=false );

	///
	/// query to see if a class id has been registered for factory new'ing.
	base_export b32 fIsFactoryRegistered( tClassId cid );


	class base_export tFactoryMap : public tHashTable<tClassId,Private::tClassAllocFunctions>
	{
		declare_singleton_define_own_ctor_dtor( tFactoryMap );

		inline tFactoryMap( ) : tHashTable<tClassId,Private::tClassAllocFunctions>( 32 ) { }
		inline ~tFactoryMap( ) { }
	};


	///
	/// Automatically registers a type for factory creation via its tClassId; factory creation
	/// means allocating a class via its tClassId, with appropriate class construction.
	template<class t, bool noOpConstruct>
	class tFactoryRegistrar
	{
	public:

		template<class tType, bool classNoOpConstruct> // the general case is the 'false' case, not requiring no opt construction
		class tClassNewFunctions
		{
		public:
			///
			/// \brief Default Rtti function for creating a new instance of type 't',
			/// using the global new operator.
			static inline void* fNewDefault( b32 noopConstruct ) 
				{ sigassert( !noopConstruct ); return NEW tType; }

			///
			/// \brief Default Rtti function for creating a new instance of type 't',
			/// using the supplied memory pointer and the in place new operator.
			static inline void* fNewInPlaceDefault( void* p, b32 noopConstruct ) 
				{ sigassert( !noopConstruct ); return new(p) tType; }
		};

		template<class tType>
		class tClassNewFunctions<tType, true>
		{
		public:
			///
			/// \brief Default Rtti function for creating a new instance of type 't',
			/// using the global new operator.
			static inline void* fNewDefault( b32 noopConstruct ) 
				{ return noopConstruct ? NEW tType( cNoOpTag ) : NEW tType; }

			///
			/// \brief Default Rtti function for creating a new instance of type 't',
			/// using the supplied memory pointer and the in place new operator.
			static inline void* fNewInPlaceDefault( void* p, b32 noopConstruct ) 
				{ return noopConstruct ? new(p) tType( cNoOpTag ) : new(p) tType; }
		};


		///
		/// \brief Accepts override functions for allocating the class 't', if desired;
		/// if no overrides are specified, uses global operator new.
		tFactoryRegistrar( )
		{
			static b32 registered = false;
			if( !registered )
			{
				registered = true;
				fRegisterFactoryClass( 
					&tClassNewFunctions<t,noOpConstruct>::fNewDefault,
					&tClassNewFunctions<t,noOpConstruct>::fNewInPlaceDefault );
			}
		}

	private:

		///
		/// \brief Registers a class for factory style instance creation.
		void fRegisterFactoryClass( Private::tClassNew cn, Private::tClassNewInPlace cnip )
		{
			const u32 cid = fGetClassId<t>( );
			log_assert( !tFactoryMap::fInstance( ).fFind( cid ), "Class ID [" << std::hex << cid << "] already registered - someone forgot to generate a new unique RttiID." );
			tFactoryMap::fInstance( ).fInsert( cid, Private::tClassAllocFunctions( cn, cnip ) ); 
		}

	};

#	define register_rtti_factory( t, requiresNoOpConstruction ) \
		namespace { static ::Sig::Rtti::tFactoryRegistrar<t,requiresNoOpConstruction> g##t##RttiFactoryRegistrar; }

#	define register_rtti_factory_scoped( t, niceName, requiresNoOpConstruction ) \
		namespace { static ::Sig::Rtti::tFactoryRegistrar<t,requiresNoOpConstruction> g##niceName##RttiFactoryRegistrar; }

}}


#endif//__tRttiFactory__
