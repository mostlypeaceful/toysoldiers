#ifndef __tFuiDataStore__
#define __tFuiDataStore__

#include "Fui.hpp"

namespace Sig
{
	namespace FUIRat
	{

		struct tMemberBase;
		typedef tRefCounterPtr< tMemberBase > tMemberBasePtr;

		struct tClassBindingData;
		typedef tRefCounterPtr< tClassBindingData > tClassBindingDataPtr;

		struct tInstance
		{
			void *mPtr;
			tClassBindingData* mProvider;

			tInstance( void* value = NULL, FUIRat::tClassBindingData* provider = NULL )
				: mPtr( value )
				, mProvider( provider )
			{ }
		};
	}

	///
	/// This singleton object represents a library of information exposed to Action script.
	class tFuiDataStore
	{
		declare_singleton_define_own_ctor_dtor( tFuiDataStore );
		tFuiDataStore( );
		~tFuiDataStore( );
	public:
		struct tGlobalInstance
		{
			tStringPtr			mName;
			FUIRat::tInstance	mInstance;

			b32 operator == ( const tStringPtr& name ) const { return mName == name; }

			tGlobalInstance( ) { }
			tGlobalInstance( const tStringPtr& name, FUIRat::tInstance& instance )
				: mName( name )
				, mInstance( instance )
			{ }
		};	

		typedef tStringPtr tStringKey;

	public:
		static void fExportFuiInterface( );

		void fRegisterProvider( const FUIRat::tClassBindingDataPtr& prov );

		template< typename t >
		FUIRat::tClassBindingData* fProvider( const t& );

		FUIRat::tClassBindingData* fProviderByName( const tStringPtr& name )
		{
			FUIRat::tClassBindingDataPtr* ptr = mProviders.fFind( name );
			return ptr ? ptr->fGetRawPtr( ) : NULL;
		}

		template< typename t >
		void fRegisterGlobalInstance( const tStringPtr& name, const t* );
		void fClearGlobalInstance( const tStringPtr& name );

		tGlobalInstance* fFindGlobalInstance( const tStringPtr& name );

		// returns true if data was set.
		b32 fSetResponseString( const std::string& str );
		b32 fSetResponseString( const std::wstring& wstr );
		b32 fSetResponseS32( s32 val );
		b32 fSetResponseF32( f32 val );
		b32 fSetResponseInstance( const FUIRat::tInstance& instance );

		enum tRequestType
		{
			cString,
			cWString,
			cS32,
			cF32,
			cInstance,
			cRequestTypeCount
		};

		void fResetResponse( u32 type ) { mResponseType = type; mSatisifed = false; }
		void fFlushResponse( const Fui::tFuiPtr& fui );

		// Debug 
		void fDumpRegisteredInstancesAndProviders( );

	private:

		u32 mResponseType;
		b32 mSatisifed;
		std::string mResponseString;
		std::wstring mResponseWString;
		s32 mResponseInt;
		f32 mResponseFloat;
		FUIRat::tInstance mResponseInstance;

		// The response values get flushed to here.
		enum tResponseVariables
		{
			cResponseString,
			cResponseWString,
			cResponseS32,
			cResponseF32,
			cResponseInstancePtr, 
			cResponseInstanceProvider,
			cResponseVariablesCount
		};

		// This stuff is for the export bindings
		tHashTable< tStringPtr, FUIRat::tClassBindingDataPtr > mProviders;
		tGrowableArray< tGlobalInstance > mGlobalInstances;
	};

	namespace FUIRat
	{

		template< typename t > struct tFunction;

		// constless
		template< class className, class R1 >
		struct tFunction < R1 (className::*)( ) >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( );
		};

		template< class className, class R1, class A1 >
		struct tFunction < R1 (className::*)( A1 ) >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1 );
		};

		template< class className, class R1, class A1, class A2 >
		struct tFunction < R1 (className::*)( A1, A2 ) >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2 );
		};

		template< class className, class R1, class A1, class A2, class A3 >
		struct tFunction < R1 (className::*)( A1, A2, A3 ) >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2, A3 );
		};

		template< class className, class R1, class A1, class A2, class A3, class A4 >
		struct tFunction < R1 (className::*)( A1, A2, A3, A4 ) >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2, A3, A4 );
		};

		// consts
		template< class className, class R1 >
		struct tFunction < R1 (className::*)( ) const >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( );
		};

		template< class className, class R1, class A1 >
		struct tFunction < R1 (className::*)( A1 ) const >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1 );
		};

		template< class className, class R1, class A1, class A2 >
		struct tFunction < R1 (className::*)( A1, A2 ) const >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2 );
		};

		template< class className, class R1, class A1, class A2, class A3 >
		struct tFunction < R1 (className::*)( A1, A2, A3 ) const >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2, A3 );
		};

		template< class className, class R1, class A1, class A2, class A3, class A4 >
		struct tFunction < R1 (className::*)( A1, A2, A3, A4 ) const >
		{
			typedef R1 tReturnType;
			typedef R1 (className::*tConstlessSig)( A1, A2, A3, A4 );
		};



		template < typename t > 
		struct tStarStripper
		{
			typedef t tType;
		};

		template< typename t >
		struct tStarStripper< t* >
		{
			typedef t tType;
		};


		/*
			tFuncArgTemp -
			 if FuiRat is trying to call a function  void fFunc( const tStringPtr& arg ),
			 There is no match for tFuiFuncParams::fGet for const tStringPtr& obviously.
			 We need to extract the real type here: tStringPtr, to use as a temporary for the fGet and fFunc call.
		*/
		template < typename t > 
		struct tFuncArgTemp
		{
			typedef t tType;
		};

		template< typename t >
		struct tFuncArgTemp< const t >
		{
			typedef t tType;
		};

		template< typename t >
		struct tFuncArgTemp< t& >
		{
			typedef t tType;
		};

		template< typename t >
		struct tFuncArgTemp< const t& >
		{
			typedef t tType;
		};



		template< typename t > struct tMemberType;

		template< typename R1, typename className >
		struct tMemberType < R1 (className::*) >
		{
			typedef R1 tType;
		};

		struct tMemberBase : tRefCounter
		{
			// The number of system parameters that proceed the function parameters.
			static const u32 cInstanceParamCount = 2; //an instance requires two parameters (ptr and provider [ie typeinfo])
			static const u32 cBaseParamCount = cInstanceParamCount + 1; // +1 for member name

			tMemberBase( )
			{ }

			virtual ~tMemberBase( )
			{ }

			struct tRequest
			{
				void* mPtr;
				Fui::tFuiFuncParams* mParams;

				tRequest( void* ptr = NULL, Fui::tFuiFuncParams* params = NULL ) 
					: mPtr( ptr )
					, mParams( params ) 
				{ }

				template< typename t >
				t* fClassRef( )
				{
					sigassert( mPtr );
					return static_cast<t*>( mPtr );
				}
			};

			virtual void fFlush( tRequest& req ) = 0
			{ 
				sigassert( !"dont get here!" );
			}

			template< typename t >
			void fStoreResult( tRequest& req, const t* value )
			{
				tClassBindingData* prov = NULL;

				if( value )
				{
					prov = tFuiDataStore::fInstance( ).fProvider( *value );
					log_assert( prov, "Member type not registered yet: " << tClassBindingData::fGetTypeName<t>( ) );
				}

				tFuiDataStore::fInstance( ).fSetResponseInstance( tInstance( const_cast<t*>( value ), prov ) );
			}

			template< typename t >
			void fStoreResult( tRequest& req, t& value )
			{
				tClassBindingData* prov = tFuiDataStore::fInstance( ).fProvider( value );
				log_assert( prov, "Member type not registered yet: " << tClassBindingData::fGetTypeName<t>( ) );

				tFuiDataStore::fInstance( ).fSetResponseInstance( tInstance( &value, prov ) );
			}

			void fStoreResult( tRequest& req, const std::string& value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value );
			}

			// Had to replace this one with the one below it.
			//  It was tripping up the following case:
			/*

				struct tSomething
				{
					tStringPtr mVal;
				};

				script export:
				.Var( "val", &tSomething::mVal )
			*/
			//void fStoreResult( tRequest& req, const tStringPtr& value )
			//{
			//	tFuiDataStore::fInstance( ).fSetResponseString( value.fCStr( ) );
			//}
			//void fStoreResult( tRequest& req, const tFilePathPtr & value )
			//{
			//	tFuiDataStore::fInstance( ).fSetResponseString( value.fCStr( ) );
			//}

			void fStoreResult( tRequest& req, tStringPtr value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value.fCStr( ) );
			}

			void fStoreResult( tRequest& req, tFilePathPtr value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value.fCStr( ) );
			}

			void fStoreResult( tRequest& req, const tLocalizedString& value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value.fToWCString( ) );
			}

			void fStoreResult( tRequest& req, const char* value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value ? value : "" );
			}

			void fStoreResult( tRequest& req, const wchar_t* value )
			{
				tFuiDataStore::fInstance( ).fSetResponseString( value ? value : L"" );
			}

			void fStoreResult( tRequest& req, u32 value )
			{
				tFuiDataStore::fInstance( ).fSetResponseS32( (s32)value );
			}

			void fStoreResult( tRequest& req, s32 value )
			{
				tFuiDataStore::fInstance( ).fSetResponseS32( (s32)value );
			}

			void fStoreResult( tRequest& req, f32 value )
			{
				tFuiDataStore::fInstance( ).fSetResponseF32( value );
			}
		};

		template< typename t >
		struct tVal
		{
			static t fToCPP( tMemberBase::tRequest& req ) 
			{
				t val;
				req.mParams->fGet( val );
				return val;
			}
		};

		template< typename tClass, typename tType >
		struct tVariable : public tMemberBase
		{
			tType mValue;

			tVariable( tType value )
				: mValue( value )
			{ }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				tClass& c = *req.fClassRef<tClass>( );

				if( req.mParams->fCount( ) > cBaseParamCount )
				{
					// set
					sigassert( req.mParams->fCount( ) == 1 + cBaseParamCount && "A variable must be called with either zero or 1 paramters. 1 parameter signifying 'set'" );
					c.*mValue = tVal< tMemberType< tType >::tType >::fToCPP( req );
				}
				else
				{
					// get
					fStoreResult( req, c.*mValue );
				}
			}
		};

		template< typename tClass, typename tFunc >
		struct tMember : public tMemberBase
		{
			tMember( /*arguement ommited on purpose*/ ); // dont let this compile. if this class is trying to be used, one of the tMember specializations is insufficent for your needs and needs to be added.
		};

		// Overload these for different number of arguement types
		// fui special, passes the tFuiFuncParams to the member.
		template< typename tClass, typename tFunc >
		struct tFuiMember : public tMemberBase
		{
			static const u32 cArgCount = 1;
			tFunc mFunc;

			tFuiMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				// No validation, up to the user.
				tClass& c = *req.fClassRef<tClass>( );
				fStoreResult( req, (c.*mFunc)( *req.mParams ) );
			}
		};

		// zero args
		// The tClass 2 is because the tFunc may be pointing to a function that only exists on a base class or something. so tClass2 will be the base class
		//  tClass will then be the owner class who is exporting the script interface
		template< typename tClass, typename tClass2, typename tReturn >
		struct tMember< tClass, tReturn (tClass2::*)() > : public tMemberBase
		{
			static const u32 cArgCount = 0;
			typedef tReturn (tClass2::*tFunc)( );
			tFunc mFunc;

			tMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				sigassert( req.mParams->fCount( ) == cArgCount + cBaseParamCount && "Parameter count mismatch." );
				tClass& c = *req.fClassRef<tClass>( );
				fStoreResult( req, (c.*mFunc)( ) );
			}
		};

		// 1 arg
		template< typename tClass, typename tClass2, typename tReturn, typename tArg1 >
		struct tMember< tClass, tReturn (tClass2::*)( tArg1 ) > : public tMemberBase
		{
			static const u32 cArgCount = 1;
			typedef typename tFuncArgTemp< tArg1 >::tType tA1;
			typedef tReturn (tClass2::*tFunc)( tArg1 );
			tFunc mFunc;

			tMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				sigassert( req.mParams->fCount( ) == cArgCount + cBaseParamCount && "Parameter count mismatch." );
				tClass& c = *req.fClassRef<tClass>( );
				tA1 a1 = tVal<tA1>::fToCPP( req );
				fStoreResult( req, (c.*mFunc)( a1 ) );
			}
		};

		// 2 arg
		template< typename tClass, typename tClass2, typename tReturn, typename tArg1, typename tArg2 >
		struct tMember< tClass, tReturn (tClass2::*)( tArg1, tArg2 ) > : public tMemberBase
		{
			static const u32 cArgCount = 2;
			typedef typename tFuncArgTemp< tArg1 >::tType tA1;
			typedef typename tFuncArgTemp< tArg2 >::tType tA2;
			typedef tReturn (tClass2::*tFunc)( tArg1, tArg2 );
			tFunc mFunc;

			tMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				sigassert( req.mParams->fCount( ) == cArgCount + cBaseParamCount && "Parameter count mismatch." );
				tClass& c = *req.fClassRef<tClass>( );
				tA1 a1 = tVal<tA1>::fToCPP( req );
				tA2 a2 = tVal<tA2>::fToCPP( req );
				fStoreResult( req, (c.*mFunc)( a1, a2 ) );
			}
		};

		// 3 arg
		template< typename tClass, typename tClass2, typename tReturn, typename tArg1, typename tArg2, typename tArg3 >
		struct tMember< tClass, tReturn (tClass2::*)( tArg1, tArg2, tArg3 ) > : public tMemberBase
		{
			static const u32 cArgCount = 3;
			typedef typename tFuncArgTemp< tArg1 >::tType tA1;
			typedef typename tFuncArgTemp< tArg2 >::tType tA2;
			typedef typename tFuncArgTemp< tArg3 >::tType tA3;
			typedef tReturn (tClass2::*tFunc)( tArg1, tArg2, tArg3 );
			tFunc mFunc;

			tMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				sigassert( req.mParams->fCount( ) == cArgCount + cBaseParamCount && "Parameter count mismatch." );
				tClass& c = *req.fClassRef<tClass>( );
				tA1 a1 = tVal<tA1>::fToCPP( req );
				tA2 a2 = tVal<tA2>::fToCPP( req );
				tA3 a3 = tVal<tA3>::fToCPP( req );
				fStoreResult( req, (c.*mFunc)( a1, a2, a3 ) );
			}
		};

		// 4 arg
		template< typename tClass, typename tClass2, typename tReturn, typename tArg1, typename tArg2, typename tArg3, typename tArg4 >
		struct tMember< tClass, tReturn (tClass2::*)( tArg1, tArg2, tArg3, tArg4 ) > : public tMemberBase
		{
			static const u32 cArgCount = 4;
			typedef typename tFuncArgTemp< tArg1 >::tType tA1;
			typedef typename tFuncArgTemp< tArg2 >::tType tA2;
			typedef typename tFuncArgTemp< tArg3 >::tType tA3;
			typedef typename tFuncArgTemp< tArg4 >::tType tA4;
			typedef tReturn (tClass2::*tFunc)( tArg1, tArg2, tArg3, tArg4 );
			tFunc mFunc;

			tMember( tFunc func ) : mFunc( func ) { }

			virtual void fFlush( tRequest& req ) OVERRIDE
			{ 
				sigassert( req.mParams->fCount( ) == cArgCount + cBaseParamCount && "Parameter count mismatch." );
				tClass& c = *req.fClassRef<tClass>( );
				tA1 a1 = tVal<tA1>::fToCPP( req );
				tA2 a2 = tVal<tA2>::fToCPP( req );
				tA3 a3 = tVal<tA3>::fToCPP( req );
				tA4 a4 = tVal<tA4>::fToCPP( req );
				fStoreResult( req, (c.*mFunc)( a1, a2, a3, a4 ) );
			}
		};


		struct tClassBindingData : tRefCounter
		{
			typedef tHashTable< tStringPtr, tMemberBasePtr > tMemberTable;

			template< typename t >
			static tStringPtr fGetTypeName( ) 
			{
				return tStringPtr( typeid( t ).name( ) );
			}

			const tStringPtr& fTypeName( ) const
			{
				return mTypeName;
			}

			tMemberBase* fMember( const tStringPtr& name )
			{
				tMemberBasePtr* ptr = mMembers.fFind( name );

				if( ptr )
					return ptr->fGetRawPtr( );
				else if( mBaseProvider )
				{
					//search the base.
					tMemberBase* baseMember = mBaseProvider->fMember( name ); 
					if( baseMember )
						return baseMember;
				}

				return NULL;
			}

			const tMemberTable& fMembers( ) const
			{
				return mMembers;
			}

			void fSetBaseClass( const tStringPtr& name )
			{
				mBaseProvider.fReset( tFuiDataStore::fInstance( ).fProviderByName( name ) );
				log_assert( mBaseProvider, "Base class needs to be registered first: " << name );
			}

			tStringPtr mTypeName;
			tMemberTable mMembers;
			tRefCounterPtr< tClassBindingData > mBaseProvider; //set if we have a base.
		};

		template< typename tClass >
		struct tDataProvider
		{			
			tDataProvider( )
			{
				mBase.fReset( NEW_TYPED( tClassBindingData )( ) );
				mBase->mTypeName = tClassBindingData::fGetTypeName<tClass>( );
			}

			template< typename tFunc >
			tDataProvider<tClass>& Func( const char* name, tFunc func )
			{
				tStringPtr namePtr( name );
				sigassert( !mBase->fMember( namePtr ) && "Member already exists!" );
				mBase->mMembers.fInsert( namePtr, tMemberBasePtr( NEW tMember<tClass, tFunction< tFunc >::tConstlessSig >( (tFunction< tFunc >::tConstlessSig)func ) ) );
				return *this;
			}

			template< typename tFunc >
			tDataProvider<tClass>& FuiFunc( const char* name, tFunc func )
			{
				tStringPtr namePtr( name );
				sigassert( !mBase->fMember( namePtr ) && "Member already exists!" );
				mBase->mMembers.fInsert( namePtr, tMemberBasePtr( NEW tFuiMember<tClass, tFunction< tFunc >::tConstlessSig >( (tFunction< tFunc >::tConstlessSig)func ) ) );
				return *this;
			}

			template< typename tFunc >
			tDataProvider<tClass>& Var( const char* name, tFunc value )
			{
				tStringPtr namePtr( name );
				sigassert( !mBase->fMember( namePtr ) && "Member already exists!" );
				mBase->mMembers.fInsert( namePtr, tMemberBasePtr( NEW tVariable<tClass, tFunc >( value ) ) );
				return *this;
			}

			void fRegister( )
			{
				tFuiDataStore::fInstance( ).fRegisterProvider( mBase );
			}
		private:
			tRefCounterPtr< tClassBindingData > mBase;
		};

		template< typename tClass, typename tBaseClass >
		struct tDerivedDataProvider : public tDataProvider<tClass>
		{			
			tDerivedDataProvider( )
			{
				fSetBaseClass( fGetTypeName<tBaseClass>( ) );
			}
		};

	}

	template< typename tClass >
	FUIRat::tClassBindingData* tFuiDataStore::fProvider( const tClass& )
	{
		tStringPtr name( FUIRat::tClassBindingData::fGetTypeName< FUIRat::tStarStripper<tClass>::tType >( ) );
		return fProviderByName( name );
	}

	template< typename t >
	void tFuiDataStore::fRegisterGlobalInstance( const tStringPtr& name, const t* inst )
	{
		sigassert( !fFindGlobalInstance( name ) && "Global instance already registered with this name." );
		FUIRat::tClassBindingData* prov = fProvider( inst );
		sigassert( prov && "Could not find provider!" );
		mGlobalInstances.fPushBack( tGlobalInstance( name, FUIRat::tInstance( (void*)inst, prov ) ) );
	}

}

#endif //__tFuiDataStore__
