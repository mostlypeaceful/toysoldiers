#ifndef __tUnitTest__
#define __tUnitTest__
#include "RttiClassHierarchy.hpp"

namespace Sig
{
	class tUnitTestException { };

	class tUnitTest : public Rtti::tClassHierarchyObject<tUnitTest>
	{
	public:

		template<class expr>
		static inline void fAssert( const expr& e ) { if( !e ) throw tUnitTestException(); }

		template<class expr>
		static inline void fAssertNull( const expr& e ) { if( e ) throw tUnitTestException(); }

		template<class expr>
		static inline void fAssertNotNull( const expr& e ) { if( !( e ) ) throw tUnitTestException(); }

		template<class avalue, class bvalue>
		static inline void fAssertEqual( const avalue& a, const bvalue& b ) { if( !fEqual( a, b ) ) throw tUnitTestException(); }

		template<class avalue, class bvalue, class evalue>
		static inline void fAssertEqualEpsilon( const avalue& a, const bvalue& b, const evalue& e ) { if( !fEqual( a, b, e ) ) throw tUnitTestException(); }

		template<class avalue, class bvalue>
		static inline void fAssertNotEqual( const avalue& a, const bvalue& b ) { if( fEqual( a, b ) ) throw tUnitTestException(); }

		template<class avalue, class bvalue, class evalue>
		static inline void fAssertNotEqualEpsilon( const avalue& a, const bvalue& b, const evalue& e ) { if( fEqual( a, b, e ) ) throw tUnitTestException(); }

	public:

		tUnitTest( Rtti::tClassId cid ) : Rtti::tClassHierarchyObject<tUnitTest>( cid ) { }

		virtual void			fExecute( ) = 0;
		virtual const char*		fGetName( ) const = 0;
	};

	template<class t>
	class tUnitTestDerived : public tUnitTest
	{
	public:
		tUnitTestDerived( ) : tUnitTest( Rtti::fGetClassId<t>( ) ) { }
	};

#define define_unittest(name) \
	class name : public ::Sig::tUnitTestDerived<name> { \
	public: \
		virtual void fExecute( ); \
		virtual const char* fGetName( ) const { return #name; } \
	} g##name; \
	void name::fExecute( )

}

#endif//__tUnitTest__

