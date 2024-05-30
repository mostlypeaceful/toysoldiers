#include "BasePch.hpp"
#include "tXmlSerializer.hpp"

namespace Sig
{

	template<>
	void fSerializeXmlObject<tXmlSerializer,int>( tXmlSerializer& s, int& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,u16>( tXmlSerializer& s, u16& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,u32>( tXmlSerializer& s, u32& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,u64>( tXmlSerializer& s, u64& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,f32>( tXmlSerializer& s, f32& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,f64>( tXmlSerializer& s, f64& object )
	{
		std::stringstream ss;
		ss << object;
		s.fCurRoot( ).fSetContents( ss.str( ).c_str( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,std::string>( tXmlSerializer& s, std::string& object )
	{
		s.fCurRoot( ).fSetContents( object.c_str( ), object.length( ) > 0 );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,tStringPtr>( tXmlSerializer& s, tStringPtr& object )
	{
		s.fCurRoot( ).fSetContents( object.fCStr( ), object.fExists( ) );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,tFilePathPtr>( tXmlSerializer& s, tFilePathPtr& object )
	{
		s.fCurRoot( ).fSetContents( object.fCStr( ), object.fExists( ) );
	}

}
